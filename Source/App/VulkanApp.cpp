#include "VulkanApp.hpp"
#include "InitVulkan.hpp"
#include "../Utils/Globals.hpp"
//stb
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//std
#include <map>
#include <set>
#include <fstream>
#include <string>
#include <chrono>

namespace cvr {


    VulkanApp::VulkanApp()
    {
		initialize(); 
    }
	void VulkanApp::initialize()
	{
		m_Window			= std::make_unique<Window>();
		m_Instance			= std::make_unique<VulkanInstance>();
		m_Surface			= std::make_unique<VulkanSurface>(m_Instance->getInstance(), m_Window->getWindow());
		m_Device			= std::make_unique<Device>(m_Instance->getInstance(), m_Surface->getSurface());
		m_Swapchain			= std::make_unique<Swapchain>(m_Device.get(), m_Surface->getSurface(), m_Window->getWindow());
		m_RenderPass		= std::make_unique<RenderPass>(m_Swapchain.get(), m_Device.get()); 
		m_UniformBuffers	= std::make_unique<UniformBuffers>(m_Device.get(), m_Swapchain.get());
		m_Texture			= std::make_unique<Texture>(m_Device.get(), m_Swapchain.get());
		m_Descriptors		= std::make_unique<DescriptorManager>(m_Device.get(), m_UniformBuffers.get(), m_Texture.get());
		m_GraphicsPipeline	= std::make_unique<GraphicsPipeline>(m_Device.get(), m_Swapchain.get(), m_Descriptors.get(), m_RenderPass.get());
		m_DepthBuffer		= std::make_unique<DepthBuffer>(m_Device.get(), m_Swapchain.get());
		m_FrameBuffer		= std::make_unique<FrameBuffer>(m_Device.get(), m_RenderPass.get(), m_DepthBuffer.get(), m_Swapchain.get());
		m_Model				= std::make_unique<Model>("Models/viking_room.obj");
		m_VertexBuffer		= std::make_unique<VertexBuffer>(m_Device.get(), m_Model->getVertices());
		m_IndexBuffer		= std::make_unique<IndexBuffer>(m_Device.get(), m_Model->getIndices());
		m_CommandBuffers	= std::make_unique<CommandBuffer>(m_Device.get(), m_RenderPass.get(), m_FrameBuffer.get(), m_Swapchain.get(), m_GraphicsPipeline.get(), m_VertexBuffer.get(), m_IndexBuffer.get(), m_Descriptors.get());
		m_SyncObjects		= std::make_unique<SyncObjects>(m_Device.get());

	}
	void VulkanApp::run()
	{
	    using clock = std::chrono::high_resolution_clock;
	    auto lastTime = clock::now();
	    int frames = 0;

	    while (!glfwWindowShouldClose(m_Window->getWindow()))
	    {
	        auto currentTime = clock::now();
	        frames++;

	        float duration = std::chrono::duration<float>(currentTime - lastTime).count();
	        if (duration >= 1.0f)
	        {
	            std::cout << "\rFPS: " << frames << std::flush;

	            frames = 0;
	            lastTime = currentTime;
	        }

	        glfwPollEvents();
	        Renderer::drawFrame(
				m_Device.get(),
				m_Swapchain.get(),
				m_SyncObjects.get(),
				m_UniformBuffers.get(),
				m_CommandBuffers.get(),
				m_Window.get(),
				m_DepthBuffer.get(),
				m_FrameBuffer.get());
	    }

	    vkDeviceWaitIdle(m_Device->getDevice());
	}



}
