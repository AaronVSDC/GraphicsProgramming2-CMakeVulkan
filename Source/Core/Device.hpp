#pragma once
#include <vector>

#include "../Utils/Structs.hpp"

#include <vulkan/vulkan.h>

//TODO: this could also just own the surface
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


		VkDevice& getDevice() { return m_Device;  }
		VkPhysicalDevice& getPhysicalDevice() { return m_PhysicalDevice;  }
		VkQueue& getPresentQueue() { return m_PresentQueue;  }
		VkQueue& getGraphicsQueue() { return m_GraphicsQueue;  }
		VkCommandPool& getCommandPool() { return m_CommandPool;  }


	private:


		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();
		bool isDeviceSuitable(VkPhysicalDevice device);

		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkQueue m_PresentQueue;
		VkQueue m_GraphicsQueue;

		VkDevice m_Device;
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkCommandPool m_CommandPool; 

		//handle ref
		VkInstance m_Instance;
		VkSurfaceKHR m_Surface; 




	};



}
