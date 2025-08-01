#pragma once
#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "Model.h"

//std 
#include <memory>
#include <vector>
#include <cassert>

namespace cve {

	class Renderer
	{
	public:
		Renderer(Window& window, Device& device);
		~Renderer();

		Renderer(const Renderer& other) = delete;
		Renderer& operator=(const Renderer& rhs) = delete;

		VkCommandBuffer BeginFrame(); 
		void EndFrame(); 
		void BeginDynamicRendering(VkCommandBuffer commandBuffer);
		void EndDynamicRendering(VkCommandBuffer commandBuffer);
		void BeginGBufferRendering(VkCommandBuffer commandBuffer);
		void EndGBufferRendering(VkCommandBuffer commandBuffer);



		VkFormat GetSwapChainImageFormat() const { return m_SwapChain->getSwapChainImageFormat(); }
		VkFormat GetDepthFormat() const { return m_SwapChain->findDepthFormat(); }
		float GetAspectRatio() const { return m_SwapChain->extentAspectRatio(); }
		uint32_t GetImageCount() const { return static_cast<uint32_t>(m_SwapChain->imageCount()); }
		VkImageView GetAlbedoImageView(uint32_t index) { return m_SwapChain->getAlbedoImageView(index); }
		VkImageView GetNormalImageView(uint32_t index) { return m_SwapChain->getNormalImageView(index); }
		SwapChain& GetSwapChain() { return *m_SwapChain; } 

		bool IsFrameInProgress() const { return m_IsFrameStarted; }
		VkCommandBuffer GetCurrentCommandBuffer() const
		{
			assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
			return m_CommandBuffers[m_CurrentFrameIndex];
		}
		int GetFrameIndex() const
		{ 
			assert(m_IsFrameStarted && "Cannot get frame index when frame not in progress :)"); 
			return m_CurrentFrameIndex;
		}
	private:
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void RecreateSwapChain();
		void TransitionImageLayout(
			VkCommandBuffer commandBuffer,
			VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			VkImageAspectFlags aspectMask);


		Window& m_Window;
		Device& m_Device;
		std::unique_ptr<SwapChain> m_SwapChain;
		std::vector<VkCommandBuffer> m_CommandBuffers;


		uint32_t m_CurrentImageIndex;
		int m_CurrentFrameIndex = 0;
		bool m_IsFrameStarted = false;
		std::vector<bool> m_ImageInitialized;
		std::vector<bool> m_DepthInitialized;
		std::vector<bool> m_GBufferInitialized;
	};

}