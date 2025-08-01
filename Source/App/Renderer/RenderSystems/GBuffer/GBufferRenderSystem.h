#pragma once
#include "Pipeline.h"
#include "Device.h"
#include "GameObject.h"
#include "Camera.h"
#include "Texture.h"

#include <memory>
#include <vector>

namespace cve
{
    class GBufferRenderSystem
	{ 
    public:
        GBufferRenderSystem(Device& device,
            const std::vector<VkFormat>& colorFormats,
            VkFormat depthFormat);
        ~GBufferRenderSystem();

        GBufferRenderSystem(const GBufferRenderSystem&) = delete;
        GBufferRenderSystem& operator=(const GBufferRenderSystem&) = delete;

        void RenderGameObjects(VkCommandBuffer commandBuffer,
            std::vector<GameObject>& gameObjects,
            const Camera& camera);
    private:
        void CreatePipelineLayout();
        void CreatePipeline(const std::vector<VkFormat>& colorFormats,
            VkFormat depthFormat);

        Device& m_Device;
        std::unique_ptr<Pipeline> m_pPipeline;
        VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
    };
}