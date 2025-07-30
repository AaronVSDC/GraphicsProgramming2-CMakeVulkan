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
		m_Window = new Window{};
		m_Instance = new VulkanInstance{};
		m_Surface = new VulkanSurface{ m_Instance->getInstance(),m_Window->getWindow() };
		m_Device = new Device{ m_Instance->getInstance(), m_Surface->getSurface() };
		m_Swapchain = new Swapchain{ m_Device, m_Surface->getSurface(), m_Window->getWindow() };
		m_RenderPass = new RenderPass{ m_Swapchain, m_Device };
		m_UniformBuffers = new UniformBuffers{ m_Device, m_Swapchain };
		m_Texture = new Texture{ m_Device, m_Swapchain };
		m_Descriptors = new DescriptorManager{ m_Device,m_UniformBuffers, m_Texture };
		m_GraphicsPipeline = new GraphicsPipeline{ m_Device, m_Swapchain, m_Descriptors, m_RenderPass };
		m_DepthBuffer = new DepthBuffer{ m_Device, m_Swapchain };
		m_FrameBuffer = new FrameBuffer{ m_Device, m_RenderPass, m_DepthBuffer, m_Swapchain };
		m_Model = new Model{ std::string{"Models/viking_room.obj"} };
		m_VertexBuffer = new VertexBuffer{ m_Device, m_Model->getVertices() };
		m_IndexBuffer = new IndexBuffer{ m_Device, m_Model->getIndices() };
		m_CommandBuffers = new CommandBuffer{ m_Device, m_RenderPass, m_FrameBuffer, m_Swapchain, m_GraphicsPipeline, m_VertexBuffer, m_IndexBuffer, m_Descriptors };
		m_SyncObjects = new SyncObjects{ m_Device };

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
	        Renderer::drawFrame(m_Device,m_Swapchain, m_SyncObjects, m_UniformBuffers, m_CommandBuffers, m_Window, m_CurrentFrame);
	    }

	    vkDeviceWaitIdle(m_Device->getDevice());
	}



}
