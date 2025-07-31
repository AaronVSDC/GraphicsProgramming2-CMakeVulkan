#include "Texture.h"
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace cve
{

	Texture::Texture(Device& device, const std::string& filename)
        : m_Device(device)
    {
        createTexture(filename);
    }

    Texture::~Texture()
    {
        vkDestroySampler(m_Device.device(), m_Sampler, nullptr);
        vkDestroyImageView(m_Device.device(), m_ImageView, nullptr);
        vkDestroyImage(m_Device.device(), m_Image, nullptr);
        vkFreeMemory(m_Device.device(), m_DeviceMemory, nullptr);
        vkDestroyDescriptorPool(m_Device.device(), s_BindlessPool, nullptr);
        vkDestroyDescriptorSetLayout(m_Device.device(), s_BindlessSetLayout, nullptr);
    }


#pragma region DESCRIPTORS

    VkDescriptorSetLayout Texture::s_BindlessSetLayout     = VK_NULL_HANDLE;
    VkDescriptorPool      Texture::s_BindlessPool          = VK_NULL_HANDLE; 
    VkDescriptorSet       Texture::s_BindlessDescriptorSet = VK_NULL_HANDLE;

    void Texture::updateBindless(Device& device, const std::vector<Texture>& textures)
    {
        uint32_t maxTextures = uint32_t(textures.size());
        std::vector<VkDescriptorImageInfo> imageInfos(maxTextures);
        std::vector<VkWriteDescriptorSet>  writes(maxTextures); 

        for (uint32_t i = 0; i < maxTextures; i++) {
            imageInfos[i].imageView = textures[i].getImageView();
            imageInfos[i].sampler = textures[i].getSampler();
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = s_BindlessDescriptorSet;
            writes[i].dstBinding = 0;
            writes[i].dstArrayElement = i;
            writes[i].descriptorCount = 1;
            writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[i].pImageInfo = &imageInfos[i];
        }

        vkUpdateDescriptorSets(device.device(),
            maxTextures,
            writes.data(),
            0, nullptr);
    }

    void Texture::initBindless(Device& device, uint32_t maxTextures)
    {
        // 1) ONE binding with an array of `maxTextures` combined-image-samplers:
        VkDescriptorSetLayoutBinding b{};
        b.binding = 0;
        b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        b.descriptorCount = maxTextures;
        b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        // 2) Enable the “variable count” & “update after bind” flags:
        VkDescriptorBindingFlags bindingFlags =
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT; 

        VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
        flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        flagsInfo.bindingCount = 1;
        flagsInfo.pBindingFlags = &bindingFlags;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT; 
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &b;
        layoutInfo.pNext = &flagsInfo;

        vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &s_BindlessSetLayout);

        // 3) Create a pool big enough for maxTextures descriptors, but only 1 set:
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = maxTextures;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &s_BindlessPool);

        // 4) Allocate the single bindless set with `maxTextures` capacity:
        VkDescriptorSetVariableDescriptorCountAllocateInfo varCountInfo{};
        varCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        varCountInfo.descriptorSetCount = 1;
        varCountInfo.pDescriptorCounts = &maxTextures;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = s_BindlessPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &s_BindlessSetLayout;
        allocInfo.pNext = &varCountInfo;

        vkAllocateDescriptorSets(device.device(), &allocInfo, &s_BindlessDescriptorSet);


    }
    void Texture::bind(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout) 
    {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &Texture::s_BindlessDescriptorSet,
            0, nullptr
        );
    }
#pragma endregion


#pragma region TEXTURE

    void Texture::createTexture(const std::string& filename)
    {
        // 1. Load m_Image pixels with stb_image
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) {
            throw std::runtime_error("Failed to load texture m_Image: " + filename);
        }

        VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4;
        m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        // 2. Create staging buffer and copy pixel data
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        m_Device.createBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
        std::memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_Device.device(), stagingBufferMemory);

        stbi_image_free(pixels);

        // 3. Create optimal-tiled VkImage
        createImage(
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight),
            m_MipLevels,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        // 4. Transition m_Image to DST for copy
        transitionImageLayout(
            m_Image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_MipLevels
        );

        // 5. Copy buffer to m_Image
        m_Device.copyBufferToImage(
            stagingBuffer,
            m_Image,
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight),
            1
        );

        // 6. Generate mipmaps on GPU
        generateMipmaps(
            m_Image,
            VK_FORMAT_R8G8B8A8_SRGB,
            texWidth, texHeight,
            m_MipLevels
        );

        // Cleanup staging resources
        vkDestroyBuffer(m_Device.device(), stagingBuffer, nullptr);
        vkFreeMemory(m_Device.device(), stagingBufferMemory, nullptr);

        // 7. Create m_Image view and m_Sampler
        createImageView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels);
        createSampler();
    }


 

    void Texture::createImage(
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        m_Device.createImageWithInfo(imageInfo, properties, m_Image, m_DeviceMemory);
    }

    void Texture::createImageView(
        VkFormat format,
        VkImageAspectFlags aspectFlags,
        uint32_t mipLevels)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_Device.device(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture m_Image view!");
        }
    }

    void Texture::createSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = m_Device.properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(m_MipLevels);

        if (vkCreateSampler(m_Device.device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture sampler!");
        }
    }

    void Texture::transitionImageLayout(
        VkImage image,
        VkFormat /*format*/,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        uint32_t mipLevels)
    {
        VkCommandBuffer cmd = m_Device.beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            cmd,
            srcStage, dstStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        m_Device.endSingleTimeCommands(cmd);
    }

    void Texture::generateMipmaps(
        VkImage image,
        VkFormat imageFormat,
        int32_t texWidth,
        int32_t texHeight,
        uint32_t mipLevels)
    {
        // Check support for linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(
            m_Device.getPhysicalDevice(), imageFormat, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("Texture m_Image format does not support linear blitting!");
        }

        VkCommandBuffer cmd = m_Device.beginSingleTimeCommands();

        for (uint32_t i = 1; i < mipLevels; i++) {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;

            // Transition previous level to SRC for blit
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            // Blit from level i-1 to level i
            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { texWidth >> (i - 1), texHeight >> (i - 1), 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;

            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { texWidth >> i, texHeight >> i, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(
                cmd,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR
            );

            // Transition level i-1 to SHADER_READ_ONLY
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }

        // Transition last mip level to SHADER_READ_ONLY
        VkImageMemoryBarrier barrierLast{};
        barrierLast.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierLast.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrierLast.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrierLast.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierLast.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierLast.image = image;
        barrierLast.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrierLast.subresourceRange.baseMipLevel = mipLevels - 1;
        barrierLast.subresourceRange.levelCount = 1;
        barrierLast.subresourceRange.baseArrayLayer = 0;
        barrierLast.subresourceRange.layerCount = 1;
        barrierLast.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrierLast.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrierLast
        );

        m_Device.endSingleTimeCommands(cmd);
    }
#pragma endregion



}