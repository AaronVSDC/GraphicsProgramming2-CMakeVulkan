#include "InitVulkan.hpp"
#include "../Utils/Globals.hpp"
//stb
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//std
#include <map>
#include <set>
#include <fstream>
#include <string>

namespace cvr {


	InitVulkan::InitVulkan()
	{
		m_Window = new Window{};
		m_Instance = new VulkanInstance{};
		m_Surface = new VulkanSurface{ m_Instance->getInstance(),m_Window->getWindow() };
		m_Device = new Device{ m_Instance->getInstance(), m_Surface->getSurface() };
		m_Swapchain = new Swapchain{ m_Device, m_Surface->getSurface(), m_Window->getWindow() };
		m_RenderPass = new RenderPass{ m_Swapchain, m_Device };
		m_UniformBuffers = new UniformBuffers{ m_Device, m_Swapchain };
		m_Texture = new Texture{ m_Device, m_Swapchain };
		m_Descriptors = new DescriptorManager{ m_Device,m_UniformBuffers, m_Texture };
		m_GraphicsPipeline = new GraphicsPipeline{ m_Device, m_Swapchain, m_Descriptors, m_RenderPass };
		m_DepthBuffer = new DepthBuffer{ m_Device, m_Swapchain };
		m_FrameBuffer = new FrameBuffer{ m_Device, m_RenderPass, m_DepthBuffer, m_Swapchain }; 
		m_Model = new Model{ MODEL_PATH }; 
		m_VertexBuffer = new VertexBuffer{ m_Device, m_Model->getVertices() };
		m_IndexBuffer = new IndexBuffer{ m_Device, m_Model->getIndices()};
		createCommandBuffers();
		createSyncObjects();
	}


	InitVulkan::~InitVulkan()
	{
		cleanup();
	}

#pragma region CLEANUP
	void InitVulkan::cleanup()
	{
		vkDeviceWaitIdle(m_Device->getDevice());

		//cleanupSwapChain();
		delete m_Swapchain;

		delete m_GraphicsPipeline;

		delete m_UniformBuffers;





		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_Device->getDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device->getDevice(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_Device->getDevice(), m_InFlightFences[i], nullptr);
		}

		delete m_RenderPass;
		delete m_Surface;
		delete m_Instance;
		delete m_Window;
		delete m_Device;
	}
#pragma endregion

#pragma region WINDOW

#pragma endregion 

#pragma region INSTANCE_CREATION
#pragma endregion

#pragma region VALIDATION_LAYERS
#pragma endregion

#pragma region PHYSICAL_DEVICE

#pragma endregion

#pragma region QUEUE_FAMILIES

#pragma endregion

#pragma region LOGICAL_DEVICE

#pragma endregion

#pragma region WINDOW_SURFACE



#pragma endregion

#pragma region SWAPCHAIN

#pragma endregion

#pragma region IMAGE_VIEWS

#pragma endregion

#pragma region GRAPHICS_PIPELINE



#pragma endregion

#pragma region RENDER_PASS


#pragma endregion    

#pragma region FRAME_BUFFER


#pragma endregion

#pragma region COMMANDPOOL_AND_COMMANDBUFFER

	void InitVulkan::createCommandBuffers()
	{
		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_Device->getCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

		if (vkAllocateCommandBuffers(m_Device->getDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

	}


	void InitVulkan::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");

		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass->getRenderPass();
		renderPassInfo.framebuffer = m_FrameBuffer->getSwapchainFrameBuffers()[imageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_Swapchain->getSwapchainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };	renderPassInfo.clearValueCount = 1;
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_Swapchain->getSwapchainExtent().width);
		viewport.height = static_cast<float>(m_Swapchain->getSwapchainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_Swapchain->getSwapchainExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->getGraphicsPipeline());

		VkBuffer vertexBuffers[] = { m_VertexBuffer->getVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->getPipelineLayout(), 0, 1, &m_Descriptors->getDescriptorSets()[m_CurrentFrame], 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_IndexBuffer->getIndices().size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

	}

#pragma endregion

#pragma region SYNC_OBJECTS

	void InitVulkan::createSyncObjects()
	{
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(m_Device->getDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_Device->getDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_Device->getDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {

				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

#pragma endregion

#pragma region DRAWING

	void InitVulkan::drawFrame()
	{
		vkWaitForFences(m_Device->getDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_Device->getDevice(), m_Swapchain->getSwapchain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			m_Swapchain->recreateSwapchain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image.");
		}

		m_UniformBuffers->updateUniformBuffer(m_CurrentFrame);

		vkResetFences(m_Device->getDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

		vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
		recordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_Swapchain->getSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		result = vkQueuePresentKHR(m_Device->getPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR or m_Window->getFrameBufferResized()) {
			m_Window->setFrameBufferResized(false);
			m_Swapchain->recreateSwapchain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image.");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}


#pragma endregion

#pragma region VERTEX_STAGING_AND_INDEX_BUFFER





	//HELPERS: 
	void InitVulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_Device->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device->getDevice(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(m_Device->getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(m_Device->getDevice(), buffer, bufferMemory, 0);
	}

	void InitVulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}



	uint32_t InitVulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_Device->getPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) and (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		throw std::runtime_error("Failed to find suitable memory type.");

	}

	VkCommandBuffer InitVulkan::beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_Device->getCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_Device->getDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void InitVulkan::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_Device->getGraphicsQueue());

		vkFreeCommandBuffers(m_Device->getDevice(), m_Device->getCommandPool(), 1, &commandBuffer);
	}
#pragma endregion

#pragma region DESCRIPTOR_SET_LAYOUT_AND_UNIFORM_BUFFERS


#pragma endregion

#pragma region DESCRIPTOR_POOL

#pragma endregion

#pragma region TEXTURES
	void InitVulkan::createImage(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(m_Device->getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_Device->getDevice(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(m_Device->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(m_Device->getDevice(), image, imageMemory, 0);
	}
#pragma endregion

#pragma region DEPTH_BUFFER



#pragma endregion

#pragma region MODEL_LOADING


#pragma endregion
}
