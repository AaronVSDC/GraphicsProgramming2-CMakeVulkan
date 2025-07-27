#pragma once

#include <iostream>
#include <vulkan/vulkan.hpp>
namespace cvr
{
	class VulkanInstance final
	{
	public: 
		VulkanInstance(); 
		~VulkanInstance(); 
		VulkanInstance(const VulkanInstance&) = delete; 
		VulkanInstance(const VulkanInstance&&) = delete; 
		VulkanInstance& operator=(const VulkanInstance&) = delete;
		VulkanInstance& operator=(const VulkanInstance&&) = delete;

		//getters/setters
		VkInstance& getInstance() { return m_Instance; }
		

	private: 
		VkInstance m_Instance;


		//-----------------------
		//VALIDATION LAYERS
		//-----------------------
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef _DEBUG
		const bool m_EnableValidationLayers = true;
#else
		const bool m_EnableValidationLayers = false;
#endif
		void setupDebugMessenger();

		bool checkValidationLayerSupport();
		std::vector<const char*> getRequiredExtension() const;

		//function to see what you want debugged (vulkan calls this function but pointer to function is setup in setupDebugMessenger)
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
			VkDebugUtilsMessageTypeFlagsEXT messageType,  
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
			void* pUserData) {
			std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl << std::endl; return VK_FALSE;
		}

		//because vkCreateDebugUtilsMessengerEXT (inside setupDebugMessenger) is an extension the address and shit must be specified which is done in this proxy function
		VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);

		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);



	};

}

