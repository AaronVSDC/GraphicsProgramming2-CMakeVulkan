#pragma once
#include "Device.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>

//std 
#include <vector>

namespace cve
{

	class Model
	{
	public:

		struct Vertex
		{
			glm::vec2 position;
			glm::vec3 color; 

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

		};


		Model(Device& device, const std::vector<Vertex>& vertices);
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		Model(const Model&&) = delete;
		Model& operator=(const Model&&) = delete;


		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);



	private:
		Device& m_Device; 
		VkBuffer m_VertexBuffer; 
		VkDeviceMemory m_VertexBufferMemory; 
		uint32_t m_VertexCount; 

		void CreateVertexBuffers(const std::vector<Vertex>& vertices); 



	};


}