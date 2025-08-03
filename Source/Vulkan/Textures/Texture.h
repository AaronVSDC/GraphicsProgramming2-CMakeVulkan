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

        //ctor for render target / G buffer attachments
        Texture(Device& device,
            uint32_t width,
            uint32_t height,
            VkFormat format,
            VkImageUsageFlags usage,
            VkImageAspectFlags aspectMask,
            bool createSampler = true);
    	~Texture();

        Texture(Texture&&) noexcept = default;
        Texture& operator=(Texture&&) noexcept = default;

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        static void bind(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout);

        static void initBindless(Device& device, uint32_t maxTextures);
        static void cleanupBindless(Device& device); 
        static void updateBindless(Device& device, const std::vector<std::unique_ptr<Texture>>& textures); 
    	static VkDescriptorSetLayout   s_BindlessSetLayout;
        static VkDescriptorPool        s_BindlessPool;
        static VkDescriptorSet         s_BindlessDescriptorSet;
        // Accessors
        VkImageView getImageView() const { return m_ImageView; }
        VkImage getImage() const { return m_Image;  }
        VkSampler   getSampler()   const { return m_Sampler; }
        uint32_t    getMipLevels() const { return m_MipLevels; }


    private:
        Device&        m_Device;
        VkImage        m_Image = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
        VkImageView    m_ImageView = VK_NULL_HANDLE;
        VkSampler      m_Sampler = VK_NULL_HANDLE;
        uint32_t       m_MipLevels = 1;



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
            int32_t texWidth,
            int32_t texHeight,
            uint32_t mipLevels
        );
    };

}