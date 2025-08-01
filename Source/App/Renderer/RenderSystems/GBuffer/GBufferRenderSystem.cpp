#include "GBufferRenderSystem.h"

#include <glm/glm.hpp>
#include <stdexcept>
#include <optional>

namespace cve
{
    struct GBufferPushConstant
	{
        glm::mat4 transform{ 1.f };
        glm::mat4 modelMatrix{ 1.f };
        uint32_t materialIndex;
        uint8_t _pad[12];
    };

    GBufferRenderSystem::GBufferRenderSystem(Device& device,
        const std::vector<VkFormat>& colorFormats,
        VkFormat depthFormat)
        : m_Device{ device } {
        CreatePipelineLayout();
        CreatePipeline(colorFormats, depthFormat);
    }

    GBufferRenderSystem::~GBufferRenderSystem() {
        vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr);
    }

    void GBufferRenderSystem::CreatePipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(GBufferPushConstant);

        VkDescriptorSetLayout setLayouts[] = { Texture::s_BindlessSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = setLayouts;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr,
            &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout for gbuffer");
        }
    }

    void GBufferRenderSystem::CreatePipeline(const std::vector<VkFormat>& colorFormats,
        VkFormat depthFormat) {
        assert(m_PipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before pipeline layout");
        PipelineConfigInfo config{};
        Pipeline::DefaultPipelineConfigInfo(config);
        config.renderPass = VK_NULL_HANDLE;
        config.colorAttachmentFormats = colorFormats;
        config.depthAttachmentFormat = depthFormat;
        config.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_EQUAL;
        config.depthStencilInfo.depthWriteEnable = VK_FALSE;
        config.pipelineLayout = m_PipelineLayout;
        m_pPipeline = std::make_unique<Pipeline>(
            m_Device,
            config,
            "Shaders/GBuffer.vert.spv",
            "Shaders/GBuffer.frag.spv");
    }

    void GBufferRenderSystem::RenderGameObjects(VkCommandBuffer commandBuffer,
        std::vector<GameObject>& gameObjects,
        const Camera& camera) {
        m_pPipeline->Bind(commandBuffer);
        Texture::bind(commandBuffer, m_PipelineLayout);

        auto pv = camera.GetProjectionMatrix() * camera.GetViewMatrix();
        for (auto& obj : gameObjects) {
            for (auto& sm : obj.m_Model->getData().submeshes) {
                GBufferPushConstant push{};
                auto model = obj.m_Transform.mat4();
                push.transform = pv * model;
                push.modelMatrix = model;
                push.materialIndex = sm.materialIndex;

                vkCmdPushConstants(commandBuffer,
                    m_PipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(push),
                    &push);
                obj.m_Model->Bind(commandBuffer);
                obj.m_Model->Draw(commandBuffer, sm.indexCount, sm.firstIndex);
            }
        }
    }
} // namespace cve