#include "Renderer.hpp"

#include <stdexcept>
#include <vulkan/vulkan.h>


namespace cvr
{

	void Renderer::drawFrame(
        Device* device,
        Swapchain* swapchain,
        SyncObjects* syncObjects,
        UniformBuffers* uniformBuffers,
        CommandBuffer* commandBuffers,
        Window* window,
        uint32_t& currentFrame
    ) {
        auto vkDevice = device->getDevice();
        auto gfxQueue = device->getGraphicsQueue();
        auto presentQ = device->getPresentQueue();
        auto& fences = syncObjects->getInFlightFences();
        auto& availSems = syncObjects->getImageAvailableSemaphores();
        auto& renderSems = syncObjects->getRenderFinishedSemaphores();
        auto& cmdBufs = commandBuffers->getCommandBuffers();
        VkSwapchainKHR  sc = swapchain->getSwapchain();

        // 1) wait for fence
        vkWaitForFences(vkDevice, 1, &fences[currentFrame], VK_TRUE, UINT64_MAX);

        // 2) acquire image
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            vkDevice, sc, UINT64_MAX,
            availSems[currentFrame], VK_NULL_HANDLE, &imageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            swapchain->recreateSwapchain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image.");
        }

        // 3) update UBO
        uniformBuffers->updateUniformBuffer(currentFrame);

        // 4) reset fence & command buffer
        vkResetFences(vkDevice, 1, &fences[currentFrame]);
        vkResetCommandBuffer(cmdBufs[currentFrame], 0);

        // 5) record commands
        commandBuffers->recordCommandBuffer(cmdBufs[currentFrame], imageIndex, currentFrame);

        // 6) submit to graphics queue
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { availSems[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBufs[currentFrame];
        VkSemaphore signalSemaphores[] = { renderSems[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(gfxQueue, 1, &submitInfo, fences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        // 7) present
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &sc;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQ, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR
            || result == VK_SUBOPTIMAL_KHR
            || window->getFrameBufferResized())
        {
            window->setFrameBufferResized(false);
            swapchain->recreateSwapchain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image.");
        }

        // 8) advance frame index
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

}
