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

		//vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT); 
		//gBuffer.m_AlbedoLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
		//gBuffer.m_DepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//gBuffer.m_NormalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//gBuffer.m_PositionLayout = VK_IMAGE_LAYOUT_UNDEFINED;

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

#pragma region FULLSCREEN_PASS
	void Renderer::BeginRenderingLighting(VkCommandBuffer commandBuffer)
	{
		assert(m_IsFrameStarted && "Cant call BeginRenderingLighting while frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame"); 


		VkImageLayout& layout = m_SwapchainImageLayouts[m_CurrentImageIndex];
		VkImageLayout desired = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (layout != desired)
		{
			TransitionImageLayout(
				commandBuffer,
				m_SwapChain->getImage(m_CurrentImageIndex),
				layout,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT);
			layout = desired; 
		}

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = m_SwapChain->getImageView(m_CurrentImageIndex);
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue.color = { 0.01f,0.01f,0.01f,1.f };


		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea.offset = { 0,0 };
		renderingInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = nullptr;

		vkCmdBeginRendering(commandBuffer, &renderingInfo);
		SetViewportAndScissor(commandBuffer); 



	}

	void Renderer::EndRenderingLighting(VkCommandBuffer commandBuffer)
	{
		assert(m_IsFrameStarted && "Cant call EndRenderingLighting while frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

		vkCmdEndRendering(commandBuffer);

		VkImageLayout& layout = m_SwapchainImageLayouts[m_CurrentImageIndex];
		VkImageLayout desired = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		if (layout != desired)
		{
			TransitionImageLayout(
				commandBuffer,
				m_SwapChain->getImage(m_CurrentImageIndex),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_IMAGE_ASPECT_COLOR_BIT);
			layout = desired; 
		}
	}
#pragma endregion

#pragma region GEOMETRY_PASS
	void Renderer::BeginRenderingGeometry(VkCommandBuffer commandBuffer, GBuffer& gBuffer)
	{
		// 1) Set up the three G-Buffer color attachments (pos, normal, albedo):
		VkRenderingAttachmentInfo cols[3]{};
		// 0) Transition all 3 G-Buffer color targets from UNDEFINED ? COLOR_ATTACHMENT_OPTIMAL:

		VkImageLayout& layoutPos = gBuffer.m_PositionLayout;
		VkImageLayout desiredPos = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (layoutPos != desiredPos)
		{
			TransitionImageLayout(
				commandBuffer,
				gBuffer.getPositionImage(),              // your helper
				layoutPos,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT
			);
			layoutPos = desiredPos;
		}

		VkImageLayout& layoutNormal = gBuffer.m_NormalLayout;
		VkImageLayout desiredNormal = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (layoutNormal != desiredNormal)
		{
			TransitionImageLayout(
				commandBuffer,
				gBuffer.getNormalImage(),
				layoutNormal,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT
			);
			layoutNormal = desiredNormal; 
		}

		VkImageLayout& layoutAlbedo = gBuffer.m_AlbedoLayout;
		VkImageLayout desiredAlbedo = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (layoutAlbedo != desiredAlbedo)
		{
			TransitionImageLayout(
				commandBuffer,
				gBuffer.getAlbedoSpecImage(),
				layoutAlbedo,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT
			);
			layoutAlbedo = desiredAlbedo; 
		}

		VkImageLayout& layoutDepth = gBuffer.m_DepthLayout;
		VkImageLayout desiredDepth = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		if (layoutDepth != desiredDepth)
		{
			TransitionImageLayout(
				commandBuffer,
				gBuffer.getDepthImage(),
				layoutDepth,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_IMAGE_ASPECT_DEPTH_BIT
			);
			layoutDepth = desiredDepth;  
		}
		// Position (RGBA16F)
		cols[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		cols[0].pNext = nullptr;
		cols[0].imageView = gBuffer.getPositionView();
		cols[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		cols[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		cols[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		cols[0].clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

		// Normal (RGBA16F)
		cols[1].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		cols[1].pNext = nullptr;
		cols[1].imageView = gBuffer.getNormalView();
		cols[1].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		cols[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		cols[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		cols[1].clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

		// Albedo(RGBA8)
		cols[2].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		cols[2].pNext = nullptr;
		cols[2].imageView = gBuffer.getAlbedoSpecView();
		cols[2].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		cols[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		cols[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		cols[2].clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

		// 2) Depth attachment
		VkRenderingAttachmentInfo depth{};
		depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depth.pNext = nullptr;
		depth.imageView = gBuffer.getDepthView();
		depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //TODO: load if you want to sample depth later
		depth.clearValue.depthStencil = { 1.0f, 0 };

		// 3) The VkRenderingInfo itself
		VkRenderingInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		info.pNext = nullptr;
		info.renderArea.offset = { 0, 0 };
		info.renderArea.extent = m_Window.GetExtent();
		info.layerCount = 1;
		info.colorAttachmentCount = 3;
		info.pColorAttachments = cols;
		info.pDepthAttachment = &depth;
		info.pStencilAttachment = nullptr;  // not using separate stencil

		// 4) Kick off the geometry pass
		vkCmdBeginRendering(commandBuffer, &info);

		VkExtent2D gExtent{ gBuffer.getWidth(), gBuffer.getHeight() };
		VkViewport vp{ 0, 0, float(gExtent.width), float(gExtent.height), 0.f, 1.f };
		vkCmdSetViewport(commandBuffer, 0, 1, &vp);
		VkRect2D sc{ {0,0}, gExtent };
		vkCmdSetScissor(commandBuffer, 0, 1, &sc);
	} 

	void Renderer::EndRenderingGeometry(VkCommandBuffer commandBuffer, GBuffer& gBuffer)
	{
		vkCmdEndRendering(commandBuffer);

		VkImageLayout& layoutPos = gBuffer.m_PositionLayout;
		VkImageLayout   desiredPos = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		if (layoutPos != desiredPos)
		{
			TransitionImageLayout(
				commandBuffer,
				gBuffer.getPositionImage(),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT
			);
			layoutPos = desiredPos; 
		}

		VkImageLayout& layoutNormal = gBuffer.m_NormalLayout; 
		VkImageLayout  desiredNormal = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		if (layoutNormal != desiredNormal)
		{
			TransitionImageLayout(
				commandBuffer,
				gBuffer.getNormalImage(),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT
			);
			layoutNormal = desiredNormal; 
		}
		VkImageLayout& layoutAlbedo = gBuffer.m_AlbedoLayout;
		VkImageLayout  desiredAlbedo = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (layoutAlbedo != desiredAlbedo)
		{
			TransitionImageLayout(
				commandBuffer,
				gBuffer.getAlbedoSpecImage(),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT
			);
			layoutAlbedo = desiredAlbedo; 
		}
	}

#pragma endregion


#pragma region DEPTH_PREPASS

	void Renderer::BeginRenderingDepthPrepass(VkCommandBuffer commandBuffer, GBuffer& gBuffer)
	{
		assert(m_IsFrameStarted && "Can't call BeginRenderingDepthPrepass while frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "Wrong command buffer");

		VkImageLayout& layout = gBuffer.m_DepthLayout;
		VkImageLayout desired = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		if (layout != desired) {
			TransitionImageLayout(
				commandBuffer,
				gBuffer.getDepthImage(),
				layout,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_IMAGE_ASPECT_DEPTH_BIT
			);
			layout = desired;
		}

		VkRenderingAttachmentInfo depthAttach{};
		depthAttach.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttach.imageView = gBuffer.getDepthView();
		depthAttach.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttach.clearValue.depthStencil = { 1.0f, 0 };

		VkRenderingInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		info.renderArea.offset = { 0, 0 };
		info.renderArea.extent = {gBuffer.getWidth(), gBuffer.getHeight()};
		info.layerCount = 1;
		info.colorAttachmentCount = 0;
		info.pColorAttachments = nullptr;
		info.pDepthAttachment = &depthAttach;
		info.pStencilAttachment = nullptr;

		vkCmdBeginRendering(commandBuffer, &info);
		VkExtent2D gExtent{ gBuffer.getWidth(), gBuffer.getHeight() };
		VkViewport vp{ 0, 0, float(gExtent.width), float(gExtent.height), 0.f, 1.f };
		vkCmdSetViewport(commandBuffer, 0, 1, &vp);
		VkRect2D sc{ {0,0}, gExtent };
		vkCmdSetScissor(commandBuffer, 0, 1, &sc);
	}

	void Renderer::EndRenderingDepthPrepass(VkCommandBuffer commandBuffer)
	{
		assert(m_IsFrameStarted && "Can't call EndRenderingDepthPrepass while frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "Wrong command buffer");
		vkCmdEndRendering(commandBuffer);
	}

#pragma endregion 
#pragma region HELPERS

	void Renderer::TransitionImageLayout(
		VkCommandBuffer commandBuffer,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageAspectFlags aspectMask)
	{
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectMask;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1; 

		VkPipelineStageFlags2 srcStage = VK_PIPELINE_STAGE_2_NONE; 
		VkPipelineStageFlags2 dstStage = VK_PIPELINE_STAGE_2_NONE;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_2_NONE;
			dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_2_NONE;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
			dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		}

		barrier.srcStageMask = srcStage; 
		barrier.dstStageMask = dstStage;
		 
		VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(commandBuffer, &depInfo);
	

	}

	void Renderer::SetViewportAndScissor(VkCommandBuffer commandBuffer)
	{
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

		m_SwapchainImageLayouts = std::vector<VkImageLayout>(
			m_SwapChain->imageCount(),
			VK_IMAGE_LAYOUT_UNDEFINED
		);
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
#pragma endregion

}
