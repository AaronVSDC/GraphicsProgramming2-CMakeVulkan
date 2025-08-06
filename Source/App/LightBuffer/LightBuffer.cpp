#include "LightBuffer.h"

namespace cve
{

    void LightBuffer::create(Device& device, uint32_t width, uint32_t height) {
        m_Width = width;
        m_Height = height;

        m_Image = std::make_unique<Texture>(
            device,
            width, height,
            HDR_FORMAT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT
        );

        m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    void LightBuffer::cleanup() {
        if (m_Image) {
            m_Image.reset();
        }
        m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

} // namespace cve