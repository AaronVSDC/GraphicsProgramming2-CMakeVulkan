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
		m_CommandBuffers = new CommandBuffer{ m_Device, m_RenderPass, m_FrameBuffer, m_Swapchain, m_GraphicsPipeline, m_VertexBuffer, m_IndexBuffer, m_Descriptors }; 
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

#pragma region COMMANDPOOL_AND_COMMANDBUFFER


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

		vkResetCommandBuffer(m_CommandBuffers->getCommandBuffers()[m_CurrentFrame], 0);
		m_CommandBuffers->recordCommandBuffer(m_CommandBuffers->getCommandBuffers()[m_CurrentFrame], imageIndex, m_CurrentFrame);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers->getCommandBuffers()[m_CurrentFrame]; 
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
}
