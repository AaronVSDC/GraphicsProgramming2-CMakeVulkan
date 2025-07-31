#pragma once
#include "Device.h"
#include <string>
#include <vulkan/vulkan.h>

namespace cve {
    class Texture {
    public:
        Texture(Device& device, const std::string& filename);
        ~Texture();

        // Accessors
        VkImageView getImageView() const { return view; }
        VkSampler   getSampler()   const { return sampler; }
        uint32_t    getMipLevels() const { return mipLevels; }

    private:
        Device&        device;
        VkImage        image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView    view = VK_NULL_HANDLE;
        VkSampler      sampler = VK_NULL_HANDLE;
        uint32_t       mipLevels = 1;

        // Helpers for image creation and transitions
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