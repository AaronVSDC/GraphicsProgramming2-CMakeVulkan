#pragma once
#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "Model.h"
#include "GBuffer.h"

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
		void BeginRenderingLighting(VkCommandBuffer commandBuffer);
		void EndRenderingLighting(VkCommandBuffer commandBuffer);
		void BeginRenderingGeometry(VkCommandBuffer commandBuffer, GBuffer& gBuffer);
		void EndRenderingGeometry(VkCommandBuffer commandBuffer, GBuffer& gBuffer); 



		VkFormat GetSwapChainImageFormat() const { return m_SwapChain->getSwapChainImageFormat(); }
		VkFormat GetDepthFormat() const { return m_SwapChain->findDepthFormat(); }
		float GetAspectRatio() const { return m_SwapChain->extentAspectRatio();  } 

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

		void SetViewportAndScissor(VkCommandBuffer commandBuffer);


		Window& m_Window;
		Device& m_Device;
		std::unique_ptr<SwapChain> m_SwapChain;
		std::vector<VkCommandBuffer> m_CommandBuffers;


		uint32_t m_CurrentImageIndex;
		int m_CurrentFrameIndex = 0;
		bool m_IsFrameStarted = false;
		std::vector<bool> m_ImageInitialized;
		std::vector<bool> m_DepthInitialized;
	};

}