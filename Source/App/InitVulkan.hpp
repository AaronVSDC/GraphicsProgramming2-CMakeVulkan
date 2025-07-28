#pragma once
#include "../Window/Window.hpp"
#include "../Core/VulkanInstance.hpp"
#include "../Window/VulkanSurface.hpp"
#include "../Utils/Structs.hpp"
#include "../Core/Device.hpp"
#include "../Core/Swapchain.hpp"

//vulkan/glfw
#include <vulkan/vulkan.h>


//std
#include <chrono>
#include <vector>
#include <iostream>


namespace cvr
{

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

		//--------
		//DRAWING
		//--------
		void drawFrame();
		uint32_t m_CurrentFrame = 0; 




		//--------------------
		//GETTERS
		//--------------------
		GLFWwindow* GetWindow() const { return m_Window->getWindow(); }
		VkDevice getDevice() const { return m_Device->getDevice();  }


		



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
		VulkanInstance* m_Instance; //includes validation layers
		VulkanSurface* m_Surface;
		Device* m_Device;
		Swapchain* m_Swapchain; 



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