#pragma once
#include "../Core/Device.hpp"
#include "../Core/SyncObjects.hpp"
#include "../Core/Swapchain.hpp"
#include "../Buffers/CommandBuffer.hpp"
#include "../Window/Window.hpp"
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
			uint32_t& currentFrame);

	private:
		uint32_t m_CurrentFrame = 0;


	};



}