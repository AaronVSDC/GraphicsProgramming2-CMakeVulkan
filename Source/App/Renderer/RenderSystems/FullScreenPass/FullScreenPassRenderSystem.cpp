#include "FullScreenPassRenderSystem.h"
#include <array>
#include <stdexcept>

namespace cve {

    FullScreenRenderSystem::FullScreenRenderSystem(Device& device, SwapChain& swapChain, VkFormat colorFormat, VkFormat depthFormat)
	: m_Device{ device }
	{
        createDescriptorSetLayout();
        createDescriptorPool(static_cast<uint32_t>(swapChain.imageCount()));
        createDescriptorSets(swapChain, static_cast<uint32_t>(swapChain.imageCount()));
        createPipelineLayout();
        createPipeline(colorFormat, depthFormat);


    }

    FullScreenRenderSystem::~FullScreenRenderSystem()
	{
        if (m_Sampler != VK_NULL_HANDLE) vkDestroySampler(m_Device.device(), m_Sampler, nullptr);
        if (m_DescriptorPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_Device.device(), m_DescriptorPool, nullptr);
        if (m_DescriptorSetLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_Device.device(), m_DescriptorSetLayout, nullptr);
        if (m_PipelineLayout != VK_NULL_HANDLE) vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr);
    }

    void FullScreenRenderSystem::createDescriptorSetLayout()
	{
        VkDescriptorSetLayoutBinding albedo{};
        albedo.binding = 0;
        albedo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        albedo.descriptorCount = 1;
        albedo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding normal = albedo;
        normal.binding = 1;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings{ albedo, normal };
        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = static_cast<uint32_t>(bindings.size());
        info.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(m_Device.device(), &info, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout for fullscreen pass");
        }
    }

    void FullScreenRenderSystem::createDescriptorPool(uint32_t frameCount)
	{
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = frameCount * 2;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = frameCount;
        if (vkCreateDescriptorPool(m_Device.device(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool for fullscreen pass");
        }
    }

    void FullScreenRenderSystem::createDescriptorSets(SwapChain& swapChain, uint32_t frameCount)
	{
        std::vector<VkDescriptorSetLayout> layouts(frameCount, m_DescriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = frameCount;
        allocInfo.pSetLayouts = layouts.data();
        m_DescriptorSets.resize(frameCount);
        if (vkAllocateDescriptorSets(m_Device.device(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets for fullscreen pass");
        }

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.maxLod = 0.0f;
        samplerInfo.maxAnisotropy = 1.0f;
        if (vkCreateSampler(m_Device.device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sampler for fullscreen pass");
        }

        for (uint32_t i = 0; i < frameCount; ++i) {
            VkDescriptorImageInfo albedoInfo{};
            albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            albedoInfo.imageView = swapChain.getAlbedoImageView(i);
            albedoInfo.sampler = m_Sampler;

            VkDescriptorImageInfo normalInfo{};
            normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            normalInfo.imageView = swapChain.getNormalImageView(i);
            normalInfo.sampler = m_Sampler;

            std::array<VkWriteDescriptorSet, 2> writes{};
            writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[0].dstSet = m_DescriptorSets[i];
            writes[0].dstBinding = 0;
            writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[0].descriptorCount = 1;
            writes[0].pImageInfo = &albedoInfo;

            writes[1] = writes[0];
            writes[1].dstBinding = 1;
            writes[1].pImageInfo = &normalInfo;

            vkUpdateDescriptorSets(m_Device.device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
        }
    }

    void FullScreenRenderSystem::createPipelineLayout() {
        VkPipelineLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount = 1;
        info.pSetLayouts = &m_DescriptorSetLayout;
        if (vkCreatePipelineLayout(m_Device.device(), &info, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout for fullscreen pass");
        }
    }

    void FullScreenRenderSystem::createPipeline(VkFormat colorFormat, VkFormat depthFormat)
	{
        PipelineConfigInfo config{};
        Pipeline::DefaultPipelineConfigInfo(config);
        config.renderPass = VK_NULL_HANDLE;
        config.colorAttachmentFormats = { colorFormat };
        config.depthAttachmentFormat = depthFormat;
        config.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        config.bindingDescriptions.clear();
        config.attributeDescriptions.clear();

        config.pipelineLayout = m_PipelineLayout;
        m_pPipeline = std::make_unique<Pipeline>(
            m_Device,
            config,
            "Shaders/FullScreenPass.vert.spv",
            "Shaders/FullScreenPass.frag.spv");
    }

    void FullScreenRenderSystem::Render(VkCommandBuffer commandBuffer, uint32_t frameIndex)
	{
        m_pPipeline->Bind(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout,
            0, 1, &m_DescriptorSets[frameIndex], 0, nullptr);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }

} // namespace cve