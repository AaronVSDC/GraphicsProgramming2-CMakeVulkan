#pragma once
#include <memory>

#include "Device.h"
#include <string>
#include <vulkan/vulkan.h>

namespace cve
{
    class Texture final
	{
    public:
        Texture(Device& device, const std::string& filename);
        ~Texture();

        void bind(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout) const;


        void allocateDescriptorSet(); 
        static void createDescriptorSetLayout(Device& device);
        static void initDescriptors(Device& device, size_t amountOfTextures);

        static std::unique_ptr<Texture> CreateTextureFromFile(Device& device, const std::string& filepath); 


        // Accessors
        VkImageView getImageView() const { return m_ImageView; }
        VkSampler   getSampler()   const { return m_Sampler; }
        uint32_t    getMipLevels() const { return m_MipLevels; }
        static VkDescriptorSetLayout& getDescriptorSetLayout() { return s_DescriptorSetLayout; }


    private:
        Device&        m_Device;
        VkImage        m_Image = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
        VkImageView    m_ImageView = VK_NULL_HANDLE;
        VkSampler      m_Sampler = VK_NULL_HANDLE;
        uint32_t       m_MipLevels = 1;
        VkDescriptorSet m_DescriptorSet;

        static VkDescriptorSetLayout s_DescriptorSetLayout;
        static VkDescriptorPool s_DescriptorPool;

    	void createTexture(const std::string& filename);

        void createImage(
            uint32_t width,
            uint32_t height,
            uint32_t mipLevels,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties
        );

        void createImageView(
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels
        );

        void createSampler();

        void transitionImageLayout(
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels
        );

        void generateMipmaps(
            VkImage image,
            VkFormat imageFormat,
            int32_t texWidth,
            int32_t texHeight,
            uint32_t mipLevels
        );
    };

}