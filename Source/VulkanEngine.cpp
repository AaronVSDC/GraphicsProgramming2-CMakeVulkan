#include "VulkanEngine.hpp"


namespace cve {

VulkanEngine::VulkanEngine()
{
}

VulkanEngine::~VulkanEngine()
{
}

void VulkanEngine::run()
{

	//main game loop
	while (!glfwWindowShouldClose(m_InitVulkan.GetWindow()))
	{
		glfwPollEvents(); 
		drawFrame(); 

	}

	vkDeviceWaitIdle(m_InitVulkan.getDevice());



}

void VulkanEngine::drawFrame()
{
	vkWaitForFences(m_InitVulkan.getDevice(), 1, &m_InitVulkan.getInFlightFence(), VK_TRUE, UINT64_MAX);
	vkResetFences(m_InitVulkan.getDevice(), 1, &m_InitVulkan.getInFlightFence());

	uint32_t imageIndex;
	vkAcquireNextImageKHR(m_InitVulkan.getDevice(), m_InitVulkan.getSwapChain(), UINT64_MAX, m_InitVulkan.getImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(m_InitVulkan.getCommandBuffer(), 0);

	m_InitVulkan.recordCommandBuffer(m_InitVulkan.getCommandBuffer(), imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_InitVulkan.getImageAvailableSemaphore() };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_InitVulkan.getCommandBuffer();
	VkSemaphore signalSemaphores[] = { m_InitVulkan.getRenderFinishedSemaphore()};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_InitVulkan.getGraphicsQueue(), 1, &submitInfo, m_InitVulkan.getInFlightFence()) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_InitVulkan.getSwapChain()};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(m_InitVulkan.getPresentQueue(), &presentInfo); 

}



}