#pragma once
#include "../Vulkan/Core/Device.hpp"
#include "../Vulkan/Core/SyncObjects.hpp"
#include "../Vulkan/Core/Swapchain.hpp"
#include "../Vulkan/Buffers/CommandBuffer.hpp"
#include "../Vulkan/Window/Window.hpp"
namespace cvr
{

	class Renderer final
	{
	public: 
		Renderer() = default; 
		~Renderer() = default; 

		static void drawFrame(
			Device* device,
			Swapchain* swapchain,
			SyncObjects* syncObjects,
			UniformBuffers* uniformBuffers,
			CommandBuffer* commandBuffers,
			Window* window,
			DepthBuffer* depthBuffer, 
			FrameBuffer* frameBuffer);

	private:
		static uint32_t m_CurrentFrame;


	};



}