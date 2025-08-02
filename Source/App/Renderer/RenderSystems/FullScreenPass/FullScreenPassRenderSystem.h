#pragma once
#include "Pipeline.h"
#include "Device.h"
#include "SwapChain.h"
#include <memory>
#include <vector>
namespace cve
{
    class FullScreenRenderSystem
	{
    public:
        FullScreenRenderSystem(Device& device, SwapChain& swapChain, VkFormat colorFormat);
        ~FullScreenRenderSystem();
        FullScreenRenderSystem(const FullScreenRenderSystem&) = delete;
        FullScreenRenderSystem& operator=(const FullScreenRenderSystem&) = delete;

        void Render(VkCommandBuffer commandBuffer, uint32_t frameIndex);

    private:
        void createDescriptorSetLayout();
        void createDescriptorPool(uint32_t frameCount);
        void createDescriptorSets(SwapChain& swapChain, uint32_t frameCount);
        void createPipelineLayout();
        void createPipeline(VkFormat colorFormat);

        Device& m_Device;
        VkDescriptorSetLayout m_DescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> m_DescriptorSets;
        VkSampler m_Sampler{ VK_NULL_HANDLE };
        std::unique_ptr<Pipeline> m_pPipeline;
        VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
    };
}