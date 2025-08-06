#pragma once 
#include "Device.h"
#include "Texture.h"
#include <memory>

namespace cve {

    class LightBuffer 
    {
    public:

        static constexpr VkFormat HDR_FORMAT = VK_FORMAT_R32G32B32A32_SFLOAT; 

        void create(Device& device, uint32_t width, uint32_t height); 
        void cleanup();

        VkImageView getImageView() const { return m_Image->getImageView(); }
        VkImage     getImage()     const { return m_Image->getImage(); }
        VkSampler   getSampler()   const { return m_Image->getSampler(); }

        VkImageLayout m_Layout{ VK_IMAGE_LAYOUT_UNDEFINED };

        uint32_t getWidth()  const { return m_Width; }
        uint32_t getHeight() const { return m_Height; }

    private:
        std::unique_ptr<Texture> m_Image;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };

} // namespace cve



