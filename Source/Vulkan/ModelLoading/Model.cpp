#include "Model.hpp"


namespace cvr
{
	void Model::loadModel(const std::string& path)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(
			path,
			aiProcess_Triangulate
			| aiProcess_FlipUVs
			| aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mMeshes) {
			throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));
		}

		aiMesh* mesh = scene->mMeshes[0];

		m_Vertices.reserve(mesh->mNumVertices);
		m_Indices.reserve(mesh->mNumFaces * 3);


		for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
			Vertex v{};

			v.pos = {
				mesh->mVertices[i].x,
				mesh->mVertices[i].y,
				mesh->mVertices[i].z
			};
			// colors (optional-OBJ doesn't carry per-vertex color by default)
			v.color = { 1.0f, 1.0f, 1.0f };

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

		for (uint32_t f = 0; f < mesh->mNumFaces; f++) {
			const aiFace& face = mesh->mFaces[f];
			for (uint32_t idx = 0; idx < face.mNumIndices; idx++) {
				m_Indices.push_back(face.mIndices[idx]);
			}
		}

	}
}