#pragma once 
#include <vector>
#include <vulkan/vulkan.h>
#include "../Utils/Globals.hpp"
#include <stdexcept>
#include "Device.hpp"

namespace cvr
{

	class SyncObjects final
	{
	public:

		SyncObjects(Device* device)
			:m_Device{ device }
		{
			createSyncObjects();
		}
		~SyncObjects()
		{
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				vkDestroySemaphore(m_Device->getDevice(), m_RenderFinishedSemaphores[i], nullptr);
				vkDestroySemaphore(m_Device->getDevice(), m_ImageAvailableSemaphores[i], nullptr);
				vkDestroyFence(m_Device->getDevice(), m_InFlightFences[i], nullptr);
			}
		}

		std::vector<VkSemaphore>& getImageAvailableSemaphores() { return  m_ImageAvailableSemaphores; }
		std::vector<VkSemaphore>& getRenderFinishedSemaphores() { return m_RenderFinishedSemaphores; }
		std::vector<VkFence>& getInFlightFences() { return m_InFlightFences; }

	private:
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		//ref
		Device* m_Device;

		void createSyncObjects()
		{
			m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				if (vkCreateSemaphore(m_Device->getDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(m_Device->getDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(m_Device->getDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create synchronization objects for a frame!");
				}
			}
		}

	}; 


}