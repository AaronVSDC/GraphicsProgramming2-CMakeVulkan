#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "../Utils/Structs.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>

namespace cvr
{
    class Model final 
	{
    public:
		Model(std::string& path)
		{
			loadModel(path); 
		}
		~Model() = default;

        const std::vector<Vertex>& getVertices() const { return m_Vertices; }
        const std::vector<uint32_t>& getIndices() const { return m_Indices; }




    private:
        std::vector<Vertex>  m_Vertices;
        std::vector<uint32_t> m_Indices;

		void loadModel(const std::string& path)
		{
			Assimp::Importer importer;

			const aiScene* scene = importer.ReadFile(
				path,
				aiProcess_Triangulate        // make sure everything is triangles
				| aiProcess_FlipUVs            // flip for GL-style UVs
				| aiProcess_CalcTangentSpace); // if you need normals/tangents

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mMeshes) {
				throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));
			}

			aiMesh* mesh = scene->mMeshes[0];  // just load the first mesh

			// Reserve so we don't reallocate
			m_Vertices.reserve(mesh->mNumVertices);
			m_Indices.reserve(mesh->mNumFaces * 3);

			// **Vertices**
			for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
				Vertex v{};
				// positions
				v.pos = {
					mesh->mVertices[i].x,
					mesh->mVertices[i].y,
					mesh->mVertices[i].z
				};
				// colors (optional-OBJ doesn't carry per-vertex color by default)
				v.color = { 1.0f, 1.0f, 1.0f };

				// texture coordinates?
				if (mesh->mTextureCoords[0]) {
					v.texCoord = {
						mesh->mTextureCoords[0][i].x,
						mesh->mTextureCoords[0][i].y
					};
				}
				else {
					v.texCoord = { 0.0f, 0.0f };
				}

				m_Vertices.push_back(v);
			}

			// **Indices** (we told Assimp to triangulate)
			for (uint32_t f = 0; f < mesh->mNumFaces; f++) {
				const aiFace& face = mesh->mFaces[f];
				for (uint32_t idx = 0; idx < face.mNumIndices; idx++) {
					m_Indices.push_back(face.mIndices[idx]);
				}
			}


		}
    };
}
