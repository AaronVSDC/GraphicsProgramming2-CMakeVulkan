#include "Model.h"
#include "Utils.h"
//libs
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm\gtx\hash.hpp>

//std
#include <cassert>
#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <iostream>

 
namespace std
{
	template <>
	struct hash<cve::Model::Vertex>
	{
		size_t operator()(cve::Model::Vertex const& vertex) const
		{
			size_t seed = 0; 
			cve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv); 
			return seed; 
		}
	};
}

namespace cve
{

	Model::Model(Device& device, Model::Data&& data)
		:m_Device{device}, m_Data{std::move(data)}
	{
		CreateVertexBuffers(m_Data.vertices); 
		CreateIndexBuffers(m_Data.indices);
	}

	Model::~Model()
	{
		vkDestroyBuffer(m_Device.device(), m_VertexBuffer, nullptr); 
		vkFreeMemory(m_Device.device(), m_VertexBufferMemory, nullptr); 

		if (m_HasIndexBuffer)
		{
			vkDestroyBuffer(m_Device.device(), m_IndexBuffer, nullptr);
			vkFreeMemory(m_Device.device(), m_IndexBufferMemory, nullptr);
		}
		Texture::cleanupBindless(m_Device);

	}


	std::unique_ptr<Model> Model::CreateModelFromFile(Device& device, const std::string& filepath) 
	{
		std::filesystem::path fp{ filepath };        
		std::string assetDir = fp.parent_path().string() + "/";

		//READ OBJ DATA
		Data data{}; 
		data.LoadModel(filepath);


		//MAKE TEXTURE FROM DATA 
		Texture::initBindless(device, data.materials.size()); 

		for (auto& mi : data.materials) 
		{
			if (!(mi.diffuseTex == "NULL"))
			{
				data.textures.emplace_back(
					std::make_unique<Texture>(device, assetDir + mi.diffuseTex)
				);
			}
			else
			{
				data.textures.emplace_back(
					std::make_unique<Texture>(device, "Resources/Missing_Texture.png")
					); 
			}

		}
		Texture::updateBindless(device, data.textures);

		return std::make_unique<Model>(device, std::move(data));
	}

	void Model::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { m_VertexBuffer }; 
		VkDeviceSize offsets[] = { 0 }; 
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets); 

		if (m_HasIndexBuffer)
		{
			vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32); 
		}

	}

	void Model::Draw(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t firstIndex)
	{
		if (m_HasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, firstIndex, 0, 0);
		}
		else
		{
			vkCmdDraw(commandBuffer, indexCount, 1, firstIndex, 0);
		}
	}

	void Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_VertexCount = static_cast<uint32_t>(vertices.size()); 
		assert(m_VertexCount >= 3 && "Vertex count must be at least 3"); 
		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount; 

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory; 
		m_Device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT ,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory); 

		void* data; 
		vkMapMemory(m_Device.device(), stagingBufferMemory, 0, bufferSize, 0, &data); 
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize)); 
		vkUnmapMemory(m_Device.device(), stagingBufferMemory); 

		m_Device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_VertexBuffer,
			m_VertexBufferMemory);

		m_Device.copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

		vkDestroyBuffer(m_Device.device(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device.device(), stagingBufferMemory, nullptr);

	}

	void Model::CreateIndexBuffers(const std::vector<uint32_t>& indices)
	{
		m_IndexCount = static_cast<uint32_t>(indices.size());
		m_HasIndexBuffer = m_IndexCount > 0; 

		if (!m_HasIndexBuffer) return; 

		VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_Device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(m_Device.device(), stagingBufferMemory);

		m_Device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_IndexBuffer,
			m_IndexBufferMemory);

		m_Device.copyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

		vkDestroyBuffer(m_Device.device(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device.device(), stagingBufferMemory, nullptr);

	}

	std::vector<VkVertexInputBindingDescription> Model::Vertex::GetBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1); 
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;   

	}

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });


		return attributeDescriptions; 


	}

	void Model::Data::LoadModel(const std::string& filepath)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(
			filepath,
			aiProcess_Triangulate        // make sure everything is triangles
			| aiProcess_FlipUVs            // flip for GL-style UVs
			| aiProcess_CalcTangentSpace); // if you need normals/tangents

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mMeshes) {
			throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));
		}

		materials.resize(scene->mNumMaterials); 
		for (size_t i = 0; i < scene->mNumMaterials; i++)
		{
			aiMaterial* mat = scene->mMaterials[i];
			aiString path;

			if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0 and mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				materials[i].diffuseTex = path.C_Str(); 
			}
			else 
			{
				
				materials[i].diffuseTex = "NULL";
			}
			//if (mat->GetTextureCount(aiTextureType_NORMALS) > 0 and mat->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
			//{
			//	materials[i].normalTex = path.C_Str();
			//}
			//TODO: add everything you want to read in (every type of texture like the two examples above)

		}

		// Calculate total counts so we can reserve memory only once
		uint32_t totalVertices = 0;
		uint32_t totalFaces = 0;
		for (uint32_t m = 0; m < scene->mNumMeshes; m++) 
		{
			totalVertices += scene->mMeshes[m]->mNumVertices;
			totalFaces += scene->mMeshes[m]->mNumFaces;
		}

		vertices.reserve(totalVertices);
		indices.reserve(totalFaces * 3);
		submeshes.reserve(scene->mNumMeshes);

		uint32_t globalVertexOffset = 0; 
		uint32_t globalIndexOffset = 0;
		// Iterate over every mesh in the file
		for (uint32_t m = 0; m < scene->mNumMeshes; m++) 
		{
			aiMesh* mesh = scene->mMeshes[m];


			//vertices
			for (uint32_t i = 0; i < mesh->mNumVertices; i++) 
			{
				Vertex v{};
				
				v.position = {
						mesh->mVertices[i].x,
						mesh->mVertices[i].y,
						mesh->mVertices[i].z
				};

				// colors (optional, is not always supported apparantly)
				v.color = { 1.0f, 1.0f, 1.0f };

				if (mesh->mTextureCoords[0]) {
					v.uv = {
							mesh->mTextureCoords[0][i].x,
							mesh->mTextureCoords[0][i].y
					};
				}
				else {
					v.uv = { 0.0f, 0.0f };
				}

				vertices.push_back(v);
			}

			//indices
			for (uint32_t f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace& face = mesh->mFaces[f];
				for (uint32_t idx = 0; idx < face.mNumIndices; idx++)
				{
					indices.push_back(face.mIndices[idx] + globalVertexOffset);
				}
			}

			//record into submesh
			uint32_t faceCount = mesh->mNumFaces;
			submeshes.push_back({
				globalIndexOffset,
				faceCount * 3,
				mesh->mMaterialIndex 
				});

			//bump offsets
			globalVertexOffset += mesh->mNumVertices;
			globalIndexOffset += faceCount * 3;

		}
	}
}