#include "GBuffer.h"

#include "SwapChain.h"

namespace cve
{
	void GBuffer::create(Device& device, uint32_t width, uint32_t height)
	{
        m_Width = width;
        m_Height = height; 

        m_PositionImage = std::make_unique<Texture>(device,
            width, height,
            POS_FORMAT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        m_NormalImage = std::make_unique<Texture>(device,
            width, height,
            NORM_FORMAT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        m_AlbedoImage = std::make_unique<Texture>(device,
            width, height,
            ALBEDO_FORMAT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        m_MetalRoughImage = std::make_unique<Texture>(device,
            width, height,
            METALROUGH_FORMAT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);
        m_MetalRoughLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        
		m_OcclusionImage = std::make_unique<Texture>(device,
                width, height,
                OCCLUSION_FORMAT, 
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
        m_OcclusionLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        // depth: no sampler needed
        m_DepthImage = std::make_unique<Texture>(device,
            width, height,
            DEPTH_FORMAT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            false);

        m_PositionLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_NormalLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
        m_AlbedoLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
        m_DepthLayout =  VK_IMAGE_LAYOUT_UNDEFINED; 

	}
    void GBuffer::cleanup() {
        // Destroy each G-buffer attachment in turn:
        if (m_PositionImage) 
            m_PositionImage.reset();
        if (m_NormalImage) 
            m_NormalImage.reset();
        if (m_AlbedoImage) 
            m_AlbedoImage.reset();
        if (m_DepthImage) 
            m_DepthImage.reset();
        if (m_MetalRoughImage)
            m_MetalRoughImage.reset();
        if (m_OcclusionImage)
            m_OcclusionImage.reset();


        // Reset layouts (optional, but keeps state clean):
        m_PositionLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_NormalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_AlbedoLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_DepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_MetalRoughLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_OcclusionLayout = VK_IMAGE_LAYOUT_UNDEFINED;  
    }
}
