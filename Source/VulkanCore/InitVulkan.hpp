#pragma once
#include "../Window/Window.hpp"
#include "Instance.hpp"


//vulkan/glfw
#include <vulkan/vulkan.h>


//glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //handy but doesnt work for nested types unfortunately (explicitly aligning is what i'll do to avoid forgetting this and causing annoying errors)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


//std
#include <chrono>
#include <vector>
#include <iostream>
#include <optional>
#include <array>

namespace cvr
{
	const int MAX_FRAMES_IN_FLIGHT = 2;

	//-----------------------------
	//MATH (vertices and stuff) 
	//-----------------------------
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}
	};

	struct UniformBufferObject
	{
		//ALWAYS LOOK UP ALIGNMENT OF DIFFERENT TYPES BECAUSE VULKAN EXPECTS
		//THEM TO BE A CERTAIN WAY INSIDE THE SHADERS
		//(im being explicit on purpose here and not including the macro
		//that does this alignment automatically when not using nested types)
		alignas(16)glm::mat4 model; 
		alignas(16)glm::mat4 view;
		alignas(16)glm::mat4 proj; 
	};



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
		GLFWwindow* GetWindow() const { return m_Window->getWindowHandle(); }
		VkDevice& getDevice() { return m_Device;  }


		



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
		void createCommandBuffers(); 
		void createSyncObjects(); 
			 
		void cleanup(); 

		//--------------------
		//REFACTORED SHIT
		//--------------------
		Window* m_Window;



		//-------------------
		//INSTANCE VARIABLES WHICH INCLUDE VALIDATION LAYERS
		//-------------------
		Instance* m_Instance; 


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
		//handled by the window


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

		//helper
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);


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

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer); 

		//---------------------------------------------
		//SYNCHRONIZATION OBJECTS
		//---------------------------------------------
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		//--------------------------
		//VERTICES/BUFFERS/UBO/...
		//--------------------------
		std::vector<Vertex> m_Vertices; 
		std::vector<uint32_t> m_Indices;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		std::vector<void*> m_UniformBuffersMapped;
		void createUniformBuffers();
		void updateUniformBuffer(uint32_t currentImage); 

		void createVertexBuffer();
		void createIndexBuffer(); 
		uint32_t findMemoryType(uint32_t typefilter, VkMemoryPropertyFlags properties); 
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory); 
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size); 


		//---------------------------------------------------------------------
		//DescriptorSets, DescriptorsetLayout, DescriptorPool
		//---------------------------------------------------------------------

		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSets();


		//------------------
		//TEXTURE LOADING 
		//------------------

		VkImage m_TextureImage;
		VkDeviceMemory m_TextureImageMemory;
		VkImageView m_TextureImageView;
		VkSampler m_TextureSampler;

		void createTextureImage();
		void createImage(uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory);
		void transitionImageLayout(VkImage image,
			VkFormat format,
			VkImageLayout oldLayout,
			VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer,
			VkImage image,
			uint32_t width,
			uint32_t height);
		void createTextureImageView(); 
		void createTextureSampler(); 


		//---------------
		// DEPTH BUFFER
		//---------------

		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView; 

		void createDepthResources();
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features); 
		VkFormat findDepthFormat();
		bool hasStencilComponent(VkFormat format);


		//--------------
		//MODEL LOADING
		//--------------
		std::string MODEL_PATH = "Models/viking_room.obj";
		std::string TEXTURE_PATH = "Textures/viking_room.png";
		void loadModel(); 

	};



}