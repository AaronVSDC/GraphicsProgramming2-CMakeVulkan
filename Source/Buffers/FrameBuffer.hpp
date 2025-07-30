#pragma once
#include "../Core/Device.hpp"
#include "../Core/Swapchain.hpp"
#include "../Graphics/RenderPass.hpp"
#include <vector>
#include "DepthBuffer.hpp"

namespace cvr
{
	class FrameBuffer final 
	{
	public:
		FrameBuffer(Device* device, RenderPass* renderPass, DepthBuffer* depthBuffer, Swapchain* swapchain)
			:m_Device{device}, m_RenderPass{renderPass}, m_DepthBuffer{depthBuffer}, m_Swapchain{swapchain}
		{
			createFrameBuffers(); 
		}
		~FrameBuffer() = default; 

		std::vector<VkFramebuffer>& getSwapchainFrameBuffers() { return m_SwapChainFramebuffers; }

	private: 

		void createFrameBuffers()
		{
			m_SwapChainFramebuffers.resize(m_Swapchain->getSwapchainImageViews().size());

			for (size_t i = 0; i < m_Swapchain->getSwapchainImageViews().size(); i++)
			{
				std::array<VkImageView, 2> attachments = {
					m_Swapchain->getSwapchainImageViews()[i],
					m_DepthBuffer->getDepthImageView()
				};

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = m_RenderPass->getRenderPass();
				framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = m_Swapchain->getSwapchainExtent().width;
				framebufferInfo.height = m_Swapchain->getSwapchainExtent().height;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer(m_Device->getDevice(), &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create framebuffer!");
				}
			}

		}

		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		//ref
		Device* m_Device; 
		Swapchain* m_Swapchain; 
		RenderPass* m_RenderPass; 
		DepthBuffer* m_DepthBuffer; 


	};

}