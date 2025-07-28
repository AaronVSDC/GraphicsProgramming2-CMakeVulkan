#pragma once
#include <vector>

#include "../Utils/Structs.hpp"

#include <vulkan/vulkan.h>


namespace cvr
{
	class Device final
	{
	public: 

		Device(VkInstance instance, VkSurfaceKHR surface); 
		~Device(); 
		
		Device(const Device&) = delete;
		Device(const Device&&) = delete;
		Device& operator=(Device&) = delete; 
		Device& operator=(Device&&) = delete;

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);


		VkDevice getDevice() const { return m_Device;  }
		VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice;  }
		VkQueue getPresentQueue() const { return m_PresentQueue;  }
		VkQueue getGraphicsQueue() const { return m_GraphicsQueue;  }


	private:


		void pickPhysicalDevice();
		void createLogicalDevice();

		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		bool isDeviceSuitable(VkPhysicalDevice device);


		VkQueue m_PresentQueue;
		VkQueue m_GraphicsQueue;

		VkDevice m_Device;
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


		//handle ref
		VkInstance m_Instance;
		VkSurfaceKHR m_Surface; 




	};



}
