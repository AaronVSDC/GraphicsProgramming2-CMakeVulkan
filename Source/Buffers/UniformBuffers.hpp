#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "../Core/Device.hpp"
#include "../Core/Swapchain.hpp"
#include "Buffer.hpp"
namespace cvr
{
	//TODO:change name to UniformBufferSet for more clarity
	class UniformBuffers final : public Buffer
	{
	public:
		UniformBuffers(Device* device, Swapchain* swapchain);
		~UniformBuffers();

		void updateUniformBuffer(uint32_t currentImage);
		std::vector<VkBuffer>& getUniformBuffers() { return m_UniformBuffers;  }


	private:

		void createUniformBuffers();

		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		std::vector<void*> m_UniformBuffersMapped;


		//ref
		Swapchain* m_Swapchain; 

	};




}
