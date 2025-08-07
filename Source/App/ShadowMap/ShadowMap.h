#pragma once
#include "Device.h"

namespace cve 
{
    class ShadowMap {
    public:
        // Create a WxH D32 image + view + compare sampler
        void create(Device& device, uint32_t width, uint32_t height);
        void cleanup();

        VkImageView getDepthView() const { return m_DepthImageView; }
        VkImage getDepthImage() const { return m_DepthImage; }
        VkSampler   getSampler()   const { return m_Sampler; }
        VkExtent2D  getExtent()    const { return { m_Width, m_Height }; }

        VkImageLayout m_Layout; 

    private: 
        Device* m_Device = nullptr;
        VkImage      m_DepthImage{};
        VkDeviceMemory m_DepthMemory{}; 
        VkImageView  m_DepthImageView{};
        VkSampler    m_Sampler{};
        uint32_t     m_Width{}, m_Height{};
    };
}
