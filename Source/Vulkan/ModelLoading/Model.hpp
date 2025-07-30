#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

#include "../Utils/Structs.hpp"
#include "../Textures/Texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <unordered_map>

namespace cvr
{

    enum class TextureType
    {
        Diffuse
    };

    struct Material
    {
        std::unordered_map<TextureType, std::unique_ptr<Texture >> textures;
        std::unordered_map<TextureType, VkDescriptorSet> descriptorSets; 

    };

    class Model final 
	{
    public:
        Model(Device* device, Swapchain* swapchain); 
		~Model() = default;

        void addTexture(TextureType type, const std::string& texturePath); 

        const std::vector<Vertex>& getVertices() const { return m_Vertices; }
        const std::vector<uint32_t>& getIndices() const { return m_Indices; }

    private:
        std::vector<Vertex>  m_Vertices;
        std::vector<uint32_t> m_Indices;

        Device* m_Device;
        Swapchain* m_Swapchain; 

        void loadModel(const std::string& path);  
    };
}
