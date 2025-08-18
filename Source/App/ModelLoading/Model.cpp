#include "Model.h"
#include "Utils.h"
//libs
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <assimp/pbrmaterial.h>    
#include <assimp/material.h>    
#define GLM_ENABLE_EXPERIMENTAL
#include <glm\gtx\hash.hpp>

//std
#include <cassert>
#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <iostream>

#include "GBuffer.h"


namespace std
{
	template <>
	struct hash<cve::Model::Vertex>
	{
		size_t operator()(cve::Model::Vertex const& vertex) const
		{
			size_t seed = 0; 
			cve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv, vertex.tangent, vertex.biTangent); 
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


		std::unordered_map<std::string, uint32_t> indexMap;
		std::vector<std::unique_ptr<Texture>>    textures;
		textures.reserve(data.materials.size() * 4); 


		auto tryLoad = [&](std::string const& filename, uint32_t& outIndex, VkFormat format)
		{
			if (filename == "NULL") 
			{
				outIndex = UINT32_MAX;
				return;
			}
			std::string full = assetDir + filename;
			auto it = indexMap.find(full);
			if (it == indexMap.end())
			{
				uint32_t idx = uint32_t(textures.size());
				indexMap[full] = idx;
				textures.emplace_back(std::make_unique<Texture>(device, full,format)); 
				outIndex = idx;
			}
			else 
			{
				outIndex = it->second;
			}
		};

		for (auto& mi : data.materials)
		{
			tryLoad(mi.baseColorTex, mi.baseColorIndex, GBuffer::ALBEDO_FORMAT);
			tryLoad(mi.metallicRoughTex, mi.metallicRoughIndex, GBuffer::METALROUGH_FORMAT);
			tryLoad(mi.normalTex, mi.normalIndex, GBuffer::NORM_FORMAT);
			tryLoad(mi.occlusionTex, mi.occlusionIndex, GBuffer::OCCLUSION_FORMAT);
		}

		data.textures = std::move(textures);
		Texture::initBindless(device, uint32_t(data.textures.size()));
		Texture::updateBindless(device, &data);
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
		attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent) });
		attributeDescriptions.push_back({ 5, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, biTangent) });

		return attributeDescriptions; 


	}

	void Model::Data::LoadModel(const std::string& filepath)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(
			filepath,
			aiProcess_Triangulate     
			| aiProcess_FlipUVs           
			| aiProcess_CalcTangentSpace
			| aiProcess_PreTransformVertices
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mMeshes) {
			throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));
		}

		// -- MATERIALS --------------------------------------------------------------

		materials.resize(scene->mNumMaterials); 
		for (size_t m = 0; m < scene->mNumMaterials; m++)
		{
			aiMaterial* mat = scene->mMaterials[m];
			auto& mi = materials[m];

			
			auto tryTex = [&](aiTextureType type, std::string& out) {
				if (mat->GetTextureCount(type) > 0) {
					aiString path;
					mat->GetTexture(type, 0, &path);
					out = path.C_Str();
				}
				};

			// BASE COLOR
			tryTex(aiTextureType_BASE_COLOR, mi.baseColorTex);

			// METALLIC-ROUGHNESS
			// Assimp may expose the combined metallic/roughness texture under
			// different texture types depending on the importer.  The original
			// code only queried aiTextureType_SPECULAR which corresponds to the
			// legacy specular/glossiness workflow and therefore failed to locate
			// the texture for glTF PBR assets such as MetalRoughSpheres.  This
			// left the metallic-roughness channel uninitialised causing the
			// spheres to render with default values.

			// First try the dedicated PBR texture types
			tryTex(aiTextureType_METALNESS, mi.metallicRoughTex);
			if (mi.metallicRoughTex == "NULL") {
				tryTex(aiTextureType_DIFFUSE_ROUGHNESS, mi.metallicRoughTex);
			}
			// Fallback for older exporters that might still use the specular slot
			if (mi.metallicRoughTex == "NULL") {
				if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
					aiString path;
					mat->GetTexture(aiTextureType_SPECULAR, 0, &path);
					mi.metallicRoughTex = path.C_Str();
				}
				else if (mat->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0) {
					aiString path;
					mat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path);
					mi.metallicRoughTex = path.C_Str();
				}
			}

			// NORMAL MAP
			if (mat->GetTextureCount(aiTextureType_NORMAL_CAMERA) > 0) {
				aiString path; mat->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &path);
				mi.normalTex = path.C_Str(); 
			}
			else {
				tryTex(aiTextureType_NORMALS, mi.normalTex);
			}

			// AMBIENT OCCLUSION
			tryTex(aiTextureType_AMBIENT_OCCLUSION, mi.occlusionTex);


			//RAW SCAN FOR PBR FACTORS (cant seem to get the macros to be recognised so doing it by hand) 
			for (unsigned int p = 0; p < mat->mNumProperties; p++) {
				auto* prop = mat->mProperties[p];
				const char* key = prop->mKey.C_Str();

				if (std::strcmp(key, "$mat.gltf.pbrMetallicRoughness.baseColorFactor") == 0
					&& prop->mDataLength >= sizeof(float) * 4) {
					auto f = reinterpret_cast<float const*>(prop->mData);
					mi.baseColorFactor = glm::vec4(f[0], f[1], f[2], f[3]);
				}
				else if (std::strcmp(key, "$mat.gltf.pbrMetallicRoughness.metallicFactor") == 0
					&& prop->mDataLength >= sizeof(float)) {
					mi.metallicFactor = *reinterpret_cast<float const*>(prop->mData);
				}
				else if (std::strcmp(key, "$mat.gltf.pbrMetallicRoughness.roughnessFactor") == 0
					&& prop->mDataLength >= sizeof(float)) {
					mi.roughnessFactor = *reinterpret_cast<float const*>(prop->mData);
				}
				else if (std::strcmp(key, "$mat.gltf.occlusionStrength") == 0
					&& prop->mDataLength >= sizeof(float)) {
					mi.occlusionStrength = *reinterpret_cast<float const*>(prop->mData);
				}
			}

			
		}

		// -- COUNT TOTAL SIZE -------------------------------------------------
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


		// -- MESHES, VERTICES, INDICES, SUBMESHES ----------------------------
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

				if (mesh->HasNormals()) {
					v.normal = {
					mesh->mNormals[i].x,
					mesh->mNormals[i].y,
					mesh->mNormals[i].z
					};
					
				}
				else {
					v.normal = { 0.0f, 0.0f, 0.0f };
				}

				
				if (mesh->HasVertexColors(0)) 
				{
					auto& c = mesh->mColors[0][i];
					v.color = { c.r, c.g, c.b };
				}
				else
				{
					v.color = { 1.0f, 1.0f, 1.0f };
				}

				if (mesh->mTextureCoords[0]) {
					v.uv = {
							mesh->mTextureCoords[0][i].x,
							mesh->mTextureCoords[0][i].y
					};
				}
				else {
					v.uv = { 0.0f, 0.0f };
				}

				if (mesh->HasTangentsAndBitangents()) {
					v.tangent = {
					  mesh->mTangents[i].x,
					  mesh->mTangents[i].y,
					  mesh->mTangents[i].z
					};
					v.biTangent = {
					  mesh->mBitangents[i].x, 
					  mesh->mBitangents[i].y,
					  mesh->mBitangents[i].z
					};
				}
				else {
					// you can orthonormalize later in the shader or generate here
					v.tangent = { 1,0,0 };
					v.biTangent = { 0,1,0 };
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