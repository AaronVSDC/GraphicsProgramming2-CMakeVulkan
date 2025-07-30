#pragma once
#include "../Window/Window.hpp"
#include "../Core/VulkanInstance.hpp"
#include "../Window/VulkanSurface.hpp"
#include "../Utils/Structs.hpp"
#include "../Core/Device.hpp"
#include "../Core/Swapchain.hpp"
#include "../Graphics/RenderPass.hpp"
#include "../Graphics/GraphicsPipeline.hpp"
#include "../Textures/Texture.hpp"
#include "../Buffers/UniformBuffers.hpp"
#include "../Buffers/DepthBuffer.hpp"
#include "../Buffers/FrameBuffer.hpp"
#include "../Buffers/VertexBuffer.hpp"
#include "../Buffers/IndexBuffer.hpp"
#include "../ModelLoading/Model.hpp"

//vulkan/glfw
#include <vulkan/vulkan.h>


//std
#include <chrono>
#include <vector>
#include "../Buffers/CommandBuffer.hpp"


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
		DescriptorManager* m_Descriptors;
		RenderPass* m_RenderPass; 
		GraphicsPipeline* m_GraphicsPipeline;
		Texture* m_Texture; 
		UniformBuffers* m_UniformBuffers;
		DepthBuffer* m_DepthBuffer;
		FrameBuffer* m_FrameBuffer; 
		VertexBuffer* m_VertexBuffer;
		Model* m_Model; 
		IndexBuffer* m_IndexBuffer; 
		CommandBuffer* m_CommandBuffers; 
		//---------------------------------------------
		//PIPELINE
		//---------------------------------------------

		//helper function to load the binary data from the shader file


		//Renderpass


		//---------------------------------------------
		//FRAMEBUFFER
		//---------------------------------------------



		//---------------------------------------------
		//COMMANDPOOL AND  COMMANDBUFFER
		//---------------------------------------------

		//VkCommandBuffer beginSingleTimeCommands();
		//void endSingleTimeCommands(VkCommandBuffer commandBuffer); 

		//---------------------------------------------
		//SYNCHRONIZATION OBJECTS
		//---------------------------------------------
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		//--------------------------
		//VERTICES/BUFFERS/UBO/...
		//--------------------------

		//uint32_t findMemoryType(uint32_t typefilter, VkMemoryPropertyFlags properties);
		//void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		//                 VkImageUsageFlags usage,
		//                 VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		//void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory); 
		//void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size); 


		//---------------------------------------------------------------------
		//DescriptorSets, DescriptorsetLayout, DescriptorPool
		//---------------------------------------------------------------------




		//------------------
		//TEXTURE LOADING 
		//------------------




		//---------------
		// DEPTH BUFFER
		//---------------


		//--------------
		//MODEL LOADING
		//--------------
		std::string MODEL_PATH = "Models/viking_room.obj";
		void loadModel(); 

	};



}