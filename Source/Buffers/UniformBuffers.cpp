#include "UniformBuffers.hpp"

#include <chrono>

#include "glm/glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "../Utils/Structs.hpp"
#include "../Utils/Globals.hpp"

namespace cvr
{

	UniformBuffers::UniformBuffers(Device* device, Swapchain* swapchain)
		:Buffer{device}, m_Swapchain{ swapchain }
	{
		createUniformBuffers(); 
	}

	UniformBuffers::~UniformBuffers()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(m_Device->getDevice(), m_UniformBuffers[i], nullptr);
			vkFreeMemory(m_Device->getDevice(), m_UniformBuffersMemory[i], nullptr);
		}

	}
	void UniformBuffers::createUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_UniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			createBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_UniformBuffers[i],
				m_UniformBuffersMemory[i]);

			vkMapMemory(m_Device->getDevice(), m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
		}

	}

	void UniformBuffers::updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(
			glm::mat4(1.0f),
			time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.view = glm::lookAt(
			glm::vec3(2.0f, 2.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.proj = glm::perspective(
			glm::radians(45.0f),
			m_Swapchain->getSwapchainExtent().width / (float)m_Swapchain->getSwapchainExtent().height,
			0.1f,
			10.0f);

		ubo.proj[1][1] *= -1;

		memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));

	}
}
