#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


//std
#include <vector>
#include <iostream>
#include <optional>

namespace cve
{
	const int MAX_FRAMES_IN_FLIGHT = 2;

	//------------------------------
	//QUEUE FAMILY STUCT
	//------------------------------
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily; 

		bool isComplete() const {
			return graphicsFamily.has_value() && presentFamily.has_value(); 
		}
	};

	//------------------------------
	//SWAPCHAIN STUCT
	//------------------------------
	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class InitVulkan final
	{
	public: 
		InitVulkan();
		~InitVulkan(); 

		InitVulkan(const InitVulkan&) = delete; 
		InitVulkan& operator=(const InitVulkan&) = delete; 
		InitVulkan(const InitVulkan&&) = delete;
		InitVulkan& operator=(const InitVulkan&&) = delete;

		//-------------------
		//COMMAND BUFFER PUBLIC METHODS
		//-------------------
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		//--------------------------
		//SWAPCHAIN PUBLIC METHODS
		//--------------------------
		void recreateSwapChain();

		//--------
		//DRAWING
		//--------
		void drawFrame();
		uint32_t m_CurrentFrame = 0; 




		//--------------------
		//GETTERS
		//--------------------
		GLFWwindow* GetWindow() const { return m_Window; }
		VkDevice& getDevice() { return m_Device;  }


		//---------
		//SETTERS
		//---------
		void setFrameBufferResized(bool newValue) { m_FrameBufferResized = newValue;  }



	private: 
		

		void initWindow(); 
		void createInstance(); 
		void setupDebugMessenger(); 
		void createSurface(); 
		void pickPhysicalDevice(); 
		void createLogicalDevice(); 
		void createSwapChain(); 
		void createImageViews(); 
		void createRenderPass(); 
		void createGraphicsPipeline(); 
		void createFrameBuffers(); 
		void createCommandPool();
		void createCommandBuffer(); 
		void createSyncObjects(); 
			 
		void cleanup(); 

		//--------------------
		//WINDOW VARIABLES
		//--------------------
		GLFWwindow* m_Window; 
		const size_t m_WINDOW_WIDTH = 1200; 
		const size_t m_WINDOW_HEIGHT = 800;
		static void frameBufferResizeCallback(GLFWwindow* window, int width, int height); 



		//-------------------
		//INSTANCE VARIABLES
		//-------------------
		VkInstance m_Instance;



		//------------------------------------------------------------------------------------
		//VALIDATION LAYER VARIABLES
		//------------------------------------------------------------------------------------
		const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" }; 

		#ifdef _DEBUG
		const bool m_EnableValidationLayers = true; 
		#else
		const bool m_EnableValidationLayers = false; 
		#endif

		bool checkValidationLayerSupport(); 
		std::vector<const char*> getRequiredExtension() const; 

		//function to see what you want debugged (vulkan calls this function but pointer to function is setup in setupDebugMessenger)
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, //specifies the severity of the message
			VkDebugUtilsMessageTypeFlagsEXT messageType,  //type of message ofc
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, //refers to VkDebugUtilsMessengerCallbackDataEXT struct containing details of the message itself
			void* pUserData) //pointer specified during setup of callback and allows you to pass your own data
		{ std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl << std::endl; return VK_FALSE; }

		//because vkCreateDebugUtilsMessengerEXT (inside setupDebugMessenger) is an extension the address and shit must be specified which is done in this proxy function
		VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger); 
		//proxy funtion to clean up
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo); 

		VkDebugUtilsMessengerEXT m_DebugMessenger; 

		//----------------------------------------------
		//PHYSICAL DEVICE VARIABLES
		//----------------------------------------------
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE; 
		bool isDeviceSuitable(VkPhysicalDevice device); 


		//---------------------------------------------
		//QUEUE FAMILY VARIABLES
		//---------------------------------------------

		//queue families is meant for finding the type of queue that we need so that the right type of commands can be submitted to the queue
		//for example we need a queue that supports graphics commands, and we need to find and use the right one ( something like that )
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device); 
		VkQueue m_PresentQueue;
		VkQueue m_GraphicsQueue;
		

		//---------------------------------------------
		//LOGICAL DEVICE
		//---------------------------------------------
		VkDevice m_Device; 
		

		//---------------------------------------------
		//WINDOW SURFACE CREATION
		//---------------------------------------------
		VkSurfaceKHR m_Surface; 

		//---------------------------------------------
		//SWAPCHAIN
		//---------------------------------------------
		VkSwapchainKHR m_SwapChain;
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; 
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device); 

		//choosing the right settings
		VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes); 
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void cleanupSwapChain(); 
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		//---------------------------------------------
		//SWAPCHAIN
		//---------------------------------------------
		std::vector<VkImageView> m_SwapChainImageViews;

		//---------------------------------------------
		//PIPELINE
		//---------------------------------------------

		//helper function to load the binary data from the shader file
		static std::vector<char> readFile(const std::string& filename); 
		VkShaderModule createShaderModule(const std::vector<char>& code) const;

		//Renderpass
		VkRenderPass m_RenderPass;
		VkPipelineLayout m_PipelineLayout;

		VkPipeline m_GraphicsPipeline;

		//---------------------------------------------
		//FRAMEBUFFER
		//---------------------------------------------

		std::vector<VkFramebuffer> swapChainFramebuffers;

		//---------------------------------------------
		//COMMANDPOOL AND  COMMANDBUFFER
		//---------------------------------------------

		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers; 



		//---------------------------------------------
		//SYNCHRONIZATION OBJECTS
		//---------------------------------------------
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		bool m_FrameBufferResized = false; 


};



}