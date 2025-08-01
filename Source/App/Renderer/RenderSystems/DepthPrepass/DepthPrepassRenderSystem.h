#pragma once
#include "Pipeline.h"
#include "Device.h"
#include "GameObject.h"
#include "Camera.h"

namespace cve 
{
    class DepthPrepassSystem final
    {
    public:
        DepthPrepassSystem(Device& device,
            const std::vector<VkFormat>& colorFormats,
            VkFormat depthFormat);
    	~DepthPrepassSystem();

        DepthPrepassSystem(const DepthPrepassSystem&) = delete;
        DepthPrepassSystem& operator=(const DepthPrepassSystem&) = delete;

        void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);
    private:
        void CreatePipelineLayout();
        void CreatePipeline(const std::vector<VkFormat>& colorFormats, VkFormat depthFormat);

        Device& m_Device;
        std::unique_ptr<Pipeline> m_pPipeline;
        VkPipelineLayout m_PipelineLayout;
    };
}

