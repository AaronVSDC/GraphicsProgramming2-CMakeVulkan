#include "Swapchain.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>


namespace cvr
{
	Swapchain::Swapchain(Device* device, VkSurfaceKHR surface, GLFWwindow* window)
		:m_Device(device), m_Surface(surface), m_Window(window)
	{
		createSwapChain();
		createImageViews(); 
	}

	Swapchain::~Swapchain()
	{
		cleanupSwapChain(); 
	}


	void Swapchain::createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = m_Device->querySwapChainSupport(m_Device->getPhysicalDevice());
		VkSurfaceFormatKHR surfaceFormat = chooseSwapChainSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		//Recommended to at least request one more image in the swapchain so that we dont have to wait on the driver to complete internal
		//operations before we can acquire another image to render to
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 and imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		//now fill in all the swapchain info
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = m_Device->findQueueFamilies(m_Device->getPhysicalDevice());
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}


		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;


		//and as always after 50 fucking years, create the swapchain
		if (vkCreateSwapchainKHR(m_Device->getDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create SwapChain ya idiot");
		}

		vkGetSwapchainImagesKHR(m_Device->getDevice(), m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device->getDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;

	}


	//EVERYTHING FOR CHOOSING THE RIGHT SETTINGS FOR THE SWAPCHAIN
	VkSurfaceFormatKHR Swapchain::chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB and availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}


		//if that fails the first one in the ranks is usually good enough.
		return availableFormats[0];
	}

	VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		//This function chooses de different presentModes, (mailbox, fifo, relaxed fifo, immediate, etc..)


		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}


		//else just fifo cause it's always supported (though not optimal if power usage isn't an issue)
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_Window, &width, &height);

			VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;

		}
	}
	void Swapchain::cleanupSwapChain()
	{
		//TODO: FIX CLEANING UP PROPERLY 

		//vkDestroyImageView(m_Device->getDevice(), m_DepthImageView, nullptr);
		//vkDestroyImage(m_Device->getDevice(), m_DepthImage, nullptr);
		//vkFreeMemory(m_Device->getDevice(), m_DepthImageMemory, nullptr);

		//for (auto framebuffer : swapChainFramebuffers)
		//{
		//	vkDestroyFramebuffer(m_Device->getDevice(), framebuffer, nullptr);
		//}
		for (auto imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_Device->getDevice(), imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_Device->getDevice(), m_SwapChain, nullptr);
	}

	void Swapchain::recreateSwapchain()
	{

		//TODO: FIX RECREATING PROPERLY
		int width = 0, height = 0;
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(m_Window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_Device->getDevice());

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		//createDepthResources();
		//createFrameBuffers();

	}


	void Swapchain::createImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
			m_SwapChainImageViews[i] = createImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}

	}
	//helper
	VkImageView Swapchain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(m_Device->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}



}

