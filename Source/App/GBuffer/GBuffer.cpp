#include "GBuffer.h"

namespace cve
{
	void GBuffer::create(Device& device, uint32_t width, uint32_t height)
	{
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

        // depth: no sampler needed
        m_DepthImage = std::make_unique<Texture>(device,
            width, height,
            DEPTH_FORMAT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            false);

	}
}