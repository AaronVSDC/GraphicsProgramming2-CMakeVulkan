#pragma once
#include "../Window/Window.hpp"
#include "../Core/VulkanInstance.hpp"
#include "../Window/VulkanSurface.hpp"
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
#include "../Core/SyncObjects.hpp"
#include "Renderer.hpp"


//namespace: CustomVulkanRenderer
namespace cvr
{
	class VulkanApp final 
	{
	public: 
		VulkanApp(); 
		~VulkanApp() = default; 

		VulkanApp(const VulkanApp&) = delete; 
		VulkanApp& operator=(const VulkanApp&) = delete; 
		VulkanApp(const VulkanApp&&) = delete;
		VulkanApp& operator=(const VulkanApp&&) = delete;



		void run(); 


	private: 

		void initialize();
		uint32_t m_CurrentFrame = 0;
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
		SyncObjects* m_SyncObjects;


	};







}



