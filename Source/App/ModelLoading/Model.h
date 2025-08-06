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
			glm::vec3 tangent;
			glm::vec3 biTangent; 

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator==(const Vertex& other) const
			{
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv && tangent == other.tangent && biTangent == other.biTangent;
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
			std::string baseColorTex = "NULL";
			std::string normalTex = "NULL";
			std::string metallicRoughTex = "NULL"; 
			std::string occlusionTex = "NULL";
			//todo; emissive texture? 

			uint32_t    baseColorIndex = UINT32_MAX;
			uint32_t    metallicRoughIndex = UINT32_MAX;
			uint32_t    normalIndex = UINT32_MAX;
			uint32_t    occlusionIndex = UINT32_MAX;

			glm::vec4 baseColorFactor{ 1,1,1,1 };
			float     metallicFactor = 1.0f;
			float     roughnessFactor = 1.0f;
			float     occlusionStrength = 1.0f;
		};


		struct Data
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
			std::vector<SubMesh> submeshes{};
			std::vector<MaterialInfo> materials{};
			std::vector<std::unique_ptr<Texture>> textures;
			void LoadModel(const std::string& filename); 
		}; 

		explicit Model(Device& device, Model::Data&& data);
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;

		static std::unique_ptr<Model> CreateModelFromFile(Device& device, const std::string& filepath); 

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t firstIndex);

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