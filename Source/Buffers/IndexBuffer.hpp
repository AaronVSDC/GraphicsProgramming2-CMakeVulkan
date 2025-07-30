#pragma once
#include "Buffer.hpp"

namespace cvr
{
	class IndexBuffer final : public Buffer
	{
	public: 
		IndexBuffer(Device* device, const std::vector<uint32_t>& indexData)
		:Buffer{device}, m_Indices{indexData}
		{
			createIndexBuffer(); 
		}

		~IndexBuffer()
		{
			vkDestroyBuffer(m_Device->getDevice(), m_IndexBuffer, nullptr);
			vkFreeMemory(m_Device->getDevice(), m_IndexBufferMemory, nullptr);
		}

		const std::vector<uint32_t>& getIndices() { return m_Indices; }
		VkBuffer& getIndexBuffer() { return m_IndexBuffer;  }

	private: 

		const std::vector<uint32_t> m_Indices;
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		void createIndexBuffer()
		{
			VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			createBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer,
				stagingBufferMemory);

			void* data; 
			vkMapMemory(m_Device->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, m_Indices.data(), (size_t)bufferSize); 
			vkUnmapMemory(m_Device->getDevice(), stagingBufferMemory);

			createBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_IndexBuffer,
				m_IndexBufferMemory);
			copyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

			vkDestroyBuffer(m_Device->getDevice(), stagingBuffer, nullptr);
			vkFreeMemory(m_Device->getDevice(), stagingBufferMemory, nullptr);
		}
	};



}