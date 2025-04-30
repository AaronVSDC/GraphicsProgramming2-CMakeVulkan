#include "Renderer.h"


//std
#include <stdexcept>
#include <array>
#include <iostream>
namespace cve {


	Renderer::Renderer(Window& window, Device& device) : m_Window{window}, m_Device{device}
	{
		RecreateSwapChain();
		CreateCommandBuffers();
	}
	Renderer::~Renderer()
	{
		FreeCommandBuffers();
	}

	void Renderer::RecreateSwapChain()
	{
		auto extent = m_Window.GetExtent();
		while (extent.width == 0 or extent.height == 0)
		{
			extent = m_Window.GetExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_Device.device());
		if (m_SwapChain == nullptr)
		{
			m_SwapChain = std::make_unique<SwapChain>(m_Device, extent);
		}
		else
		{
			std::shared_ptr<SwapChain> oldSwapChain = std::move(m_SwapChain); 
			m_SwapChain = std::make_unique<SwapChain>(m_Device, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*m_SwapChain.get()))
			{
				throw std::runtime_error("Swap chain image or depth has changed :)");
			}
		}
	}
	void Renderer::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_Device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

		if (vkAllocateCommandBuffers(m_Device.device(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers (Renderer class)");
		}
	}

	void Renderer::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(m_Device.device(),
			m_Device.getCommandPool(),
			static_cast<uint32_t>(m_CommandBuffers.size()),
			m_CommandBuffers.data());
		m_CommandBuffers.clear();
	}

	VkCommandBuffer Renderer::BeginFrame()
	{
		assert(!m_IsFrameStarted && "Can't call BeginFrame while already in progress"); 

		auto result = m_SwapChain->acquireNextImage(&m_CurrentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image");
		}

		m_IsFrameStarted = true; 

		auto commandBuffer = GetCurrentCommandBuffer(); 

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer");
		}
		return commandBuffer; 
	}

	void Renderer::EndFrame()
	{
		assert(m_IsFrameStarted && "Cant call EndFrame while frame is not in progress"); 
		auto commandBuffer = GetCurrentCommandBuffer(); 

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer");
		}
		auto result = m_SwapChain->submitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR or m_Window.WasWindowResized())
		{
			m_Window.ResetWindowResizedFlag();
			RecreateSwapChain();

		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image");
		}

		m_IsFrameStarted = false; 
		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT; 
	}

	void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_IsFrameStarted && "Cant call BeginSwapChainRenderPass while frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame"); 


		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_SwapChain->getRenderPass();
		renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer(m_CurrentImageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f,0.01f,0.01f,1.f }; //background color
		clearValues[1].depthStencil = { 1.f,0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();


		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>(m_SwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_SwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		VkRect2D scissor{ {0,0}, m_SwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);



	}

	void Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_IsFrameStarted && "Cant call EndSwapChainRenderPass while frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

		vkCmdEndRenderPass(commandBuffer);

	}
}