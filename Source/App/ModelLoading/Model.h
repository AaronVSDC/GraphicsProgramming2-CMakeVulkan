#pragma once
#include "Device.h"
#include "Texture.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>

//std 
#include <memory>
#include <vector>

namespace cve
{

	class Model
	{
	public:

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color; 
			glm::vec3 normal{}; 
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator==(const Vertex& other) const
			{
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv; 
			}
		};

		struct SubMesh
		{
			uint32_t firstIndex;
			uint32_t indexCount;
			uint32_t materialIndex; 
		};

		struct MaterialInfo
		{
			std::string diffuseTex;
			std::string normalTex;

			//TODO: add textureTypes you want to load in
		};

		struct Data
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
			std::vector<SubMesh> submeshes{};
			std::vector<MaterialInfo> materials{};
			std::vector<Texture> textures{};

			void LoadModel(const std::string& filename); 
		}; 

		Model(Device& device, const Model::Data& data);
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;

		static std::unique_ptr<Model> CreateModelFromFile(Device& device, const std::string& filepath); 

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

		Data& getData() { return m_Data;  };



	private:
		Device& m_Device; 
		VkBuffer m_VertexBuffer; 
		VkDeviceMemory m_VertexBufferMemory; 
		uint32_t m_VertexCount;

		bool m_HasIndexBuffer = false; 
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;
		uint32_t m_IndexCount;

		Data m_Data; 

		void CreateVertexBuffers(const std::vector<Vertex>& vertices); 
		void CreateIndexBuffers(const std::vector<uint32_t>& indices);



	};


}