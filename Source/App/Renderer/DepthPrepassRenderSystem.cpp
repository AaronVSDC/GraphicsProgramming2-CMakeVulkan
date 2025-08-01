#include "DepthPrepassRenderSystem.h"

#include <glm/glm.hpp>
#include <stdexcept> 
#include <optional>


namespace cve 
{

    struct PrepassPushConstantData
    {
        glm::mat4 transform{ 1.f }; 
    };

    DepthPrepassSystem::DepthPrepassSystem(Device& device, VkFormat colorFormat, VkFormat depthFormat) 
        : m_Device{ device }
    {
        CreatePipelineLayout();
        CreatePipeline(colorFormat, depthFormat); 
    }

    DepthPrepassSystem::~DepthPrepassSystem()
    {
        vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr); 
    }

    void DepthPrepassSystem::CreatePipelineLayout() 
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PrepassPushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout for depth prepass");
        }
    }

    void DepthPrepassSystem::CreatePipeline(VkFormat colorFormat, VkFormat depthFormat)
    {
        assert(m_PipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before layout");
        PipelineConfigInfo config{};
        Pipeline::DefaultPipelineConfigInfo(config);
        config.colorAttachmentFormats = { colorFormat };
        config.colorBlendAttachment.colorWriteMask = 0;
        config.colorBlendInfo.attachmentCount = 1;
        config.colorBlendInfo.pAttachments = &config.colorBlendAttachment;
        config.depthAttachmentFormat = depthFormat;
        config.pipelineLayout = m_PipelineLayout;

        m_pPipeline = std::make_unique<Pipeline>(
            m_Device,
            config,
            "Shaders/DepthPrepass.vert.spv",
            std::nullopt);
    }

    void DepthPrepassSystem::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera) 
    {
        m_pPipeline->Bind(commandBuffer);

        auto pv = camera.GetProjectionMatrix() * camera.GetViewMatrix();
        for (auto& obj : gameObjects) {
            for (auto& sm : obj.m_Model->getData().submeshes) {
                PrepassPushConstantData push{};
                push.transform = pv * obj.m_Transform.mat4();

                vkCmdPushConstants(commandBuffer,
                    m_PipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof(push),
                    &push);
                obj.m_Model->Bind(commandBuffer);
                obj.m_Model->Draw(commandBuffer, sm.indexCount, sm.firstIndex);
            }
        }
    }

} // namespace cve
