#pragma once
#include "Buffer.hpp"

namespace cvr
{

	class VertexBuffer final : public Buffer
	{
	public: 
		VertexBuffer(Device* device, const std::vector<Vertex>& vertexData)
			:Buffer{device}, m_Vertices{vertexData}
		{
			createVertexBuffer(); 
		}

		~VertexBuffer()
		{
			vkDestroyBuffer(m_Device->getDevice(), m_VertexBuffer, nullptr);
			vkFreeMemory(m_Device->getDevice(), m_VertexBufferMemory, nullptr);
		}

		const std::vector<Vertex>& getVertices() { return m_Vertices; }
		VkBuffer& getVertexBuffer() { return m_VertexBuffer;  }

	private: 
		const std::vector<Vertex> m_Vertices;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;

		void createVertexBuffer()
		{
			VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			createBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer,
				stagingBufferMemory);

			void* data;
			vkMapMemory(m_Device->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, m_Vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(m_Device->getDevice(), stagingBufferMemory);

			createBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_VertexBuffer,
				m_VertexBufferMemory);

			copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

			vkDestroyBuffer(m_Device->getDevice(), stagingBuffer, nullptr);
			vkFreeMemory(m_Device->getDevice(), stagingBufferMemory, nullptr);


		}
	};
}