#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "../Utils/Structs.hpp"
#include "Device.hpp"

#include "GLFW/glfw3.h"


namespace cvr
{
	class Swapchain
	{
	public: 
		Swapchain(Device* device, VkSurfaceKHR surface, GLFWwindow* window); 
		~Swapchain(); 
		Swapchain(const Swapchain&) = delete; 
		Swapchain(const Swapchain&&) = delete; 
		Swapchain& operator=(const Swapchain&) = delete; 
		Swapchain& operator=(const Swapchain&&) = delete;

		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		void recreateSwapchain();

		VkSwapchainKHR getSwapchain() const { return m_SwapChain;  }
		VkExtent2D getSwapchainExtent() const { return m_SwapChainExtent;  }
		VkFormat getSwapchainImageFormat() const { return m_SwapChainImageFormat;  }
		std::vector<VkImageView> getSwapchainImageViews() const { return m_SwapChainImageViews;  }

	private: 

		
		VkSwapchainKHR m_SwapChain;
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		std::vector<VkImageView> m_SwapChainImageViews;

		Device* m_Device;
		VkSurfaceKHR m_Surface;
		GLFWwindow* m_Window; 

		void createSwapChain();
		void createImageViews();


		//choosing the right settings
		VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void cleanupSwapChain();





	};



}


