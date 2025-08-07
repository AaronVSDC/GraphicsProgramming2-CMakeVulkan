#include "HDRImage.h"
#include <array>
#include <filesystem>
#include <stdexcept>

#include "Pipeline.h"
#include "stb_image.h"
#include "Texture.h"

namespace cve
{
	HDRImage::HDRImage(Device& device, const std::string& filename)
		:m_Device{device}
	{
        // 1) Load the HDR pixels with stb_image
        if (!std::filesystem::exists(filename)) {
            throw std::runtime_error("File does not exist: " + filename);
        }
        if (!stbi_is_hdr(filename.c_str())) {
            throw std::runtime_error("File is not HDR format: " + filename);
        }

        int texWidth, texHeight, texChannels;
        float* pixels = stbi_loadf(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) {
            throw std::runtime_error("Failed to load HDR image: " + filename);
        }
        m_EquirectMipLevels = 1; // only one mip level for now
        m_EquirectExtent = { uint32_t(texWidth), uint32_t(texHeight) };
        m_EquirectFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

        VkDeviceSize imageSize = VkDeviceSize(texWidth) * texHeight * 4 * sizeof(float);

        // 2) Create a host?visible staging buffer
        VkBuffer       stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        m_Device.createBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );

        // 3) Copy pixels into the staging buffer
        void* data;
        vkMapMemory(m_Device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
        std::memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_Device.device(), stagingBufferMemory);
        stbi_image_free(pixels);

        // 4) Create the equirectangular image (device?local)
        CreateEquirectImage(
            texWidth,
            texHeight,
            m_EquirectMipLevels,
            m_EquirectFormat,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
        );
        CreateEquirectTextureImageView();

        // 5) Transition image to TRANSFER_DST_OPTIMAL, copy, then to SHADER_READ_ONLY_OPTIMAL
        TransitionImageLayout(
            m_EquirectImage,
            m_EquirectFormat,
            m_EquirectImageLayout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_EquirectMipLevels
        );

        m_Device.copyBufferToImage(
            stagingBuffer,
            m_EquirectImage,
            texWidth,
            texHeight,
            1
        );

        TransitionImageLayout(
            m_EquirectImage,
            m_EquirectFormat,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            m_EquirectMipLevels
        );
        m_EquirectImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // 6) Cleanup staging
        vkDestroyBuffer(m_Device.device(), stagingBuffer, nullptr);
        vkFreeMemory(m_Device.device(), stagingBufferMemory, nullptr);

        // 7) Create the sampler
        CreateEquirectTextureSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

        // 8) Build the cube?map from this equirectangular image
        CreateCubeMap();
	}
	HDRImage::~HDRImage()
	{
        vkDestroySampler(m_Device.device(), m_EquirectSampler, nullptr);
        vkDestroyImageView(m_Device.device(), m_EquirectImageView, nullptr);
        vkDestroyImage(m_Device.device(), m_EquirectImage, nullptr);
        vkFreeMemory(m_Device.device(), m_EquirectImageMemory, nullptr);

        vkDestroySampler(m_Device.device(), m_CubeMapSampler, nullptr);
        vkDestroyImageView(m_Device.device(), m_CubeMapImageView, nullptr);
        for (auto& faceViews : m_CubeMapFaceViews)
            for (auto v : faceViews)
                vkDestroyImageView(m_Device.device(), v, nullptr);
        vkDestroyImage(m_Device.device(), m_CubeMapImage, nullptr);
        vkFreeMemory(m_Device.device(), m_CubeMapImageMemory, nullptr);
	}
    void HDRImage::CreateEquirectImage(
        uint32_t          width,
        uint32_t          height,
        uint32_t          miplevels,
        VkFormat          format,
        VkImageUsageFlags usage)
    {
        // 1) Fill out the VkImageCreateInfo
        VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = miplevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // 2) Use your Device helper to create the image and allocate/bind its memory
        //    (under the hood this does vkCreateImage + vkAllocateMemory + vkBindImageMemory)
        m_Device.createImageWithInfo( 
            imageInfo,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_EquirectImage,
            m_EquirectImageMemory   // <-- store the VkDeviceMemory here
        );
    }

    void HDRImage::CreateEquirectTextureImageView()
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_EquirectImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_EquirectFormat;
        viewInfo.subresourceRange.aspectMask = GetImageAspect(m_EquirectFormat);
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = m_EquirectMipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_Device.device(), &viewInfo, nullptr, &m_EquirectImageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image view!");
        }
    }

    void HDRImage::CreateEquirectTextureSampler(VkFilter filter, VkSamplerAddressMode addressMode)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        samplerInfo.magFilter = filter;
        samplerInfo.minFilter = filter;
        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = m_Device.properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f; 

        if (vkCreateSampler(m_Device.device(), &samplerInfo, nullptr, &m_EquirectSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture sampler!");
        }
    }

    VkImageAspectFlags HDRImage::GetImageAspect(VkFormat format)
    {
        switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    void HDRImage::CreateCubeMap()
    {
        // We only need one mip level for now
        uint32_t cubeMipLevels = 1;

        // 1. Create the VkImage
        VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_CubeMapExtent.width;
        imageInfo.extent.height = m_CubeMapExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = cubeMipLevels;
        imageInfo.arrayLayers = m_FACE_COUNT;
        imageInfo.format = m_EquirectFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT
            | VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        // Allocate and bind memory via your Device wrapper
        // You'll need a member VkDeviceMemory m_CubeMapImageMemory;
        m_Device.createImageWithInfo(
            imageInfo,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_CubeMapImage,
            m_CubeMapImageMemory
        );

        // 2. Create the cube?map view
        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = m_CubeMapImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.format = m_EquirectFormat;
        viewInfo.subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT, // aspectMask
            0,                         // baseMipLevel
            1,                         // levelCount
            0,                         // baseArrayLayer
            m_FACE_COUNT              // layerCount
        };
        if (vkCreateImageView(m_Device.device(), &viewInfo, nullptr, &m_CubeMapImageView) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create cubemap image view!");
        }

        // 3. Create per?face 2D views (for rendering each face)
        for (uint32_t face = 0; face < m_FACE_COUNT; ++face) {
            VkImageViewCreateInfo faceViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            faceViewInfo.image = m_CubeMapImage;
            faceViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            faceViewInfo.format = m_EquirectFormat;
            faceViewInfo.subresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT, // aspectMask
                0,                         // baseMipLevel
                1,                         // levelCount
                face,                      // baseArrayLayer
                1                          // layerCount
            };

            VkImageView faceView;
            if (vkCreateImageView(m_Device.device(), &faceViewInfo, nullptr, &faceView) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create cubemap face view!");
            }
            m_CubeMapFaceViews[face].push_back(faceView);
        }

        // 4. Create the sampler
        VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = m_Device.properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(cubeMipLevels);
        samplerInfo.mipLodBias = 0.0f;

        if (vkCreateSampler(m_Device.device(), &samplerInfo, nullptr, &m_CubeMapSampler) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create cubemap sampler!");
        }

        // 5. Finally render into it
        RenderToCubeMap(
            m_CubeMapExtent,
            cubeMipLevels,
            m_CubeVertPath,
            m_SkyFragPath,
            m_EquirectImage,
            m_EquirectImageView,
            m_EquirectSampler,
            m_CubeMapImage,
            m_CubeMapFaceViews
        );
    }

	void HDRImage::RenderToCubeMap(const VkExtent2D& extent,
		uint32_t mipLevels,
		const std::string& vertPath,
		const std::string& fragPath,
		VkImage& inputImage,
		const VkImageView& inputImageView,
		VkSampler inputSampler,
		VkImage& outputCubeMapImage,
		std::array<std::vector<VkImageView>, 6>& outputCubeMapImageViews)
	{
        // 1) Transition the equirectangular input to SHADER_READ_ONLY_OPTIMAL if needed
        if (m_EquirectImageLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            TransitionImageLayout(
                inputImage,
                m_EquirectFormat,
                m_EquirectImageLayout,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                m_EquirectMipLevels);
            m_EquirectImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        // 2) Begin one-off command buffer
        VkCommandBuffer cmd = m_Device.beginSingleTimeCommands();

        // 3) Transition the entire cube-map image into COLOR_ATTACHMENT_OPTIMAL
        {
            VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            barrier.srcAccessMask = 0;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.image = outputCubeMapImage;
            barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, m_FACE_COUNT };

            VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
            dep.imageMemoryBarrierCount = 1;
            dep.pImageMemoryBarriers = &barrier;
            vkCmdPipelineBarrier2(cmd, &dep);
        }

        // 4) Build a one-off descriptor set to sample the equirectangular map
        VkDescriptorSetLayout descriptorLayout;
        {
            VkDescriptorSetLayoutBinding b{ 0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                nullptr
            };
            VkDescriptorSetLayoutCreateInfo li{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
            li.bindingCount = 1;
            li.pBindings = &b;
            vkCreateDescriptorSetLayout(m_Device.device(), &li, nullptr, &descriptorLayout);
        }

        VkDescriptorPool descriptorPool;
        {
            VkDescriptorPoolSize sz{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };
            VkDescriptorPoolCreateInfo pi{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
            pi.maxSets = 1;
            pi.poolSizeCount = 1;
            pi.pPoolSizes = &sz;
            vkCreateDescriptorPool(m_Device.device(), &pi, nullptr, &descriptorPool);
        }

        VkDescriptorSet descriptorSet;
        {
            VkDescriptorSetAllocateInfo ai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
            ai.descriptorPool = descriptorPool;
            ai.descriptorSetCount = 1;
            ai.pSetLayouts = &descriptorLayout;
            vkAllocateDescriptorSets(m_Device.device(), &ai, &descriptorSet);

            VkDescriptorImageInfo ii{};
            ii.sampler = inputSampler;
            ii.imageView = inputImageView;
            ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet w{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            w.dstSet = descriptorSet;
            w.dstBinding = 0;
            w.dstArrayElement = 0;
            w.descriptorCount = 1;
            w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            w.pImageInfo = &ii;
            vkUpdateDescriptorSets(m_Device.device(), 1, &w, 0, nullptr);
        }

        // 5) Create pipeline layout with 2×mat4 push-constants + our single descriptor set
        VkPipelineLayout pipelineLayout;
        {
            VkPushConstantRange pc{};
            pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pc.offset = 0;
            pc.size = sizeof(glm::mat4) * 2;  // view + projection

            VkPipelineLayoutCreateInfo pli{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
            pli.setLayoutCount = 1;
            pli.pSetLayouts = &descriptorLayout;
            pli.pushConstantRangeCount = 1;
            pli.pPushConstantRanges = &pc;
            vkCreatePipelineLayout(m_Device.device(), &pli, nullptr, &pipelineLayout);
        }

        // 6) Configure and build a minimal pipeline
        PipelineConfigInfo cfg{};
        Pipeline::DefaultPipelineConfigInfo(cfg);
        cfg.vertexBindings.clear();    // no vertex buffers
        cfg.vertexAttributes.clear();
        cfg.colorAttachmentFormats = { m_EquirectFormat };
        cfg.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        cfg.pipelineLayout = pipelineLayout;
        cfg.renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        cfg.renderingInfo.colorAttachmentCount = 1;
        cfg.renderingInfo.pColorAttachmentFormats = cfg.colorAttachmentFormats.data();

        auto pipeline = std::make_unique<Pipeline>(
            m_Device,
            cfg,
            vertPath,
            fragPath
        );

        // 7) Prepare capture projection / views
        const glm::mat4 captureProj = [] {
            glm::mat4 p = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            p[1][1] *= -1.0f;  // GLM coordinate flip
            return p;
            }();

            const glm::vec3 eye{ 0.0f,0.0f,0.0f };
            const std::array<glm::mat4, 6> captureViews = {
                glm::lookAt(eye, eye + glm::vec3(1, 0, 0), glm::vec3(0,-1, 0)),
                glm::lookAt(eye, eye + glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)),
                glm::lookAt(eye, eye + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
                glm::lookAt(eye, eye + glm::vec3(0,-1, 0), glm::vec3(0, 0,-1)),
                glm::lookAt(eye, eye + glm::vec3(0, 0, 1), glm::vec3(0,-1, 0)),
                glm::lookAt(eye, eye + glm::vec3(0, 0,-1), glm::vec3(0,-1, 0))
            };

            // 8) Render each of the six faces
            for (uint32_t face = 0; face < m_FACE_COUNT; ++face) {
                // (a) barrier for this layer is already handled above by the 6-layer barrier

                // (b) dynamic rendering begin
                VkRenderingAttachmentInfo colorAtt{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
                colorAtt.imageView = outputCubeMapImageViews[face][0];
                colorAtt.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                colorAtt.clearValue.color = { 0,0,0,1 };

                VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO };
                ri.renderArea.extent = extent;
                ri.layerCount = 1;
                ri.colorAttachmentCount = 1;
                ri.pColorAttachments = &colorAtt;

                vkCmdBeginRendering(cmd, &ri);

                // (c) set viewport & scissor
                VkViewport vp{ 0,0, float(extent.width), float(extent.height), 0,1 };
                VkRect2D  sc{ {0,0}, extent };
                vkCmdSetViewport(cmd, 0, 1, &vp);
                vkCmdSetScissor(cmd, 0, 1, &sc);

                // (d) bind pipeline + descriptor
                pipeline->Bind(cmd);
                vkCmdBindDescriptorSets(
                    cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 0, 1, &descriptorSet,
                    0, nullptr
                );

                // (e) push constants: view then proj
                vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
                    0, sizeof(glm::mat4), &captureViews[face]);
                vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
                    sizeof(glm::mat4), sizeof(glm::mat4), &captureProj);

                // (f) draw a 36-vertex cube (generated in your vert shader)
                vkCmdDraw(cmd, 36, 1, 0, 0);

                vkCmdEndRendering(cmd);
            }

            // 9) finally transition the cube to SHADER_READ_ONLY_OPTIMAL
            {
                VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 }; 
                barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
                barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.image = outputCubeMapImage;
                barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, m_FACE_COUNT };

                VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
                dep.imageMemoryBarrierCount = 1;
                dep.pImageMemoryBarriers = &barrier;
                vkCmdPipelineBarrier2(cmd, &dep);
            }

            // 10) End and submit
            m_Device.endSingleTimeCommands(cmd);

            // 11) Clean up
            vkDestroyPipelineLayout(m_Device.device(), pipelineLayout, nullptr);
            vkDestroyDescriptorPool(m_Device.device(), descriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(m_Device.device(), descriptorLayout, nullptr);
	}

    void HDRImage::TransitionImageLayout(
        VkImage image,
        VkFormat /*format*/,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        uint32_t mipLevels)
    {
        VkCommandBuffer cmd = m_Device.beginSingleTimeCommands();

        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags2 srcStage;
        VkPipelineStageFlags2 dstStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        }
        else {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        barrier.srcStageMask = srcStage;
        barrier.dstStageMask = dstStage;

        VkDependencyInfo depInfo{};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(cmd, &depInfo);

        m_Device.endSingleTimeCommands(cmd);
    }



}
