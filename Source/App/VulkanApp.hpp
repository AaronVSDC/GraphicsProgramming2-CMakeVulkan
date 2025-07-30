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
		std::unique_ptr<Window>           m_Window;
		std::unique_ptr<VulkanInstance>   m_Instance;
		std::unique_ptr<VulkanSurface>    m_Surface;
		std::unique_ptr<Device>           m_Device;
		std::unique_ptr<Swapchain>        m_Swapchain;
		std::unique_ptr<RenderPass>       m_RenderPass;
		std::unique_ptr<UniformBuffers>   m_UniformBuffers;
		std::unique_ptr<Texture>          m_Texture;
		std::unique_ptr<DescriptorManager> m_Descriptors;
		std::unique_ptr<GraphicsPipeline> m_GraphicsPipeline;
		std::unique_ptr<DepthBuffer>      m_DepthBuffer;
		std::unique_ptr<FrameBuffer>      m_FrameBuffer;
		std::unique_ptr<Model>            m_Model;
		std::unique_ptr<VertexBuffer>     m_VertexBuffer;
		std::unique_ptr<IndexBuffer>      m_IndexBuffer;
		std::unique_ptr<CommandBuffer>    m_CommandBuffers;
		std::unique_ptr<SyncObjects>      m_SyncObjects;


	};







}



