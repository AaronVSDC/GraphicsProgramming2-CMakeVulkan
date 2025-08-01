#pragma once

#include "Device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>
namespace cve {

	class SwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		SwapChain(Device& deviceRef, VkExtent2D windowExtent);
		SwapChain(Device& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);

		~SwapChain();

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		VkImageView getImageView(int index) { return swapChainImageViews[index]; }
		VkImageView getDepthImageView(int index) { return depthImageViews[index]; }
		VkImageView getAlbedoImageView(int index) { return gAlbedoImageViews[index]; }
		VkImageView getNormalImageView(int index) { return gNormalImageViews[index]; }
		VkImage getImage(int index) { return swapChainImages[index]; }
		VkImage getDepthImage(int index) { return depthImages[index]; }
		VkImage getAlbedoImage(int index) { return gAlbedoImages[index]; }
		VkImage getNormalImage(int index) { return gNormalImages[index]; }
		size_t imageCount() { return swapChainImages.size(); } 
		VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
		VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
		uint32_t width() const { return swapChainExtent.width; }
		uint32_t height() const { return swapChainExtent.height; }

		float extentAspectRatio() const {
			return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
		}
		VkFormat findDepthFormat();

		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		bool compareSwapFormats(const SwapChain& swapChain) const
		{
			return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
				swapChain.swapChainImageFormat == swapChainImageFormat;
		}

	private:
		void init();
		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createGBufferResources();
		void createSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkFormat swapChainImageFormat;
		VkFormat swapChainDepthFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemorys;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkImage> gAlbedoImages;
		std::vector<VkDeviceMemory> gAlbedoImageMemorys;
		std::vector<VkImageView> gAlbedoImageViews;
		std::vector<VkImage> gNormalImages;
		std::vector<VkDeviceMemory> gNormalImageMemorys;
		std::vector<VkImageView> gNormalImageViews; 

		Device& device;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapChain;
		std::shared_ptr<SwapChain> oldSwapChain;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;
	};

}  // namespace lve
