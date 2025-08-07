#include "ShadowMap.h"

#include <stdexcept>

namespace cve
{

    void ShadowMap::create(Device& device, uint32_t width, uint32_t height)
	{
        m_Device = &device; m_Width = width; m_Height = height;
        // 1) Create depth image
        VkImageCreateInfo imgInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imgInfo.imageType = VK_IMAGE_TYPE_2D;
        imgInfo.format = VK_FORMAT_D32_SFLOAT;
        imgInfo.extent = { width, height, 1 };
        imgInfo.mipLevels = 1;
        imgInfo.arrayLayers = 1;
        imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imgInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT;
        imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        m_Device->createImageWithInfo(
            imgInfo,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_DepthImage,
            m_DepthMemory); 
        // allocate & bind memory...
        // 2) Image view
        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = m_DepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_D32_SFLOAT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;

    	if (vkCreateImageView(device.device(), &viewInfo, nullptr, &m_DepthImageView) != VK_SUCCESS)
            throw std::runtime_error("could not create ShadowMap imageView. ");
        
        // 3) Compare sampler for PCF
        VkSamplerCreateInfo sampInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        sampInfo.magFilter = VK_FILTER_LINEAR;
        sampInfo.minFilter = VK_FILTER_LINEAR;
        sampInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        sampInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampInfo.compareEnable = VK_TRUE;
        sampInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        if (vkCreateSampler(device.device(), &sampInfo, nullptr, &m_Sampler) != VK_SUCCESS)
            throw std::runtime_error("could not create ShadowMap sampler. ");

    }

    void ShadowMap::cleanup()
    {
        if (m_Sampler != VK_NULL_HANDLE)
            vkDestroySampler(m_Device->device(), m_Sampler, nullptr);
        if (m_DepthImageView != VK_NULL_HANDLE)
            vkDestroyImageView(m_Device->device(), m_DepthImageView, nullptr);
        if (m_DepthImage != VK_NULL_HANDLE)
            vkDestroyImage(m_Device->device(), m_DepthImage, nullptr);
        if (m_DepthMemory != VK_NULL_HANDLE)
            vkFreeMemory(m_Device->device(), m_DepthMemory, nullptr);

        m_Sampler = VK_NULL_HANDLE;
        m_DepthImageView = VK_NULL_HANDLE;
        m_DepthImage = VK_NULL_HANDLE;
        m_DepthMemory = VK_NULL_HANDLE;
        m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

}
