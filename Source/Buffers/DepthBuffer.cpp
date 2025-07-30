#include "DepthBuffer.hpp"
#include "../Textures/Texture.hpp"
namespace cvr
{
	DepthBuffer::DepthBuffer(Device* device, Swapchain* swapchain)
		:m_Device{ device }, m_Swapchain{ swapchain }
	{
		createDepthResources();
	}
	DepthBuffer::~DepthBuffer()
	{

		cleanup(); 
	}

	void DepthBuffer::cleanup()
	{
		vkDestroyImageView(m_Device->getDevice(), m_DepthImageView, nullptr);
		vkDestroyImage(m_Device->getDevice(), m_DepthImage, nullptr);
		vkFreeMemory(m_Device->getDevice(), m_DepthImageMemory, nullptr);

		
	}
	void DepthBuffer::recreate()
	{
		cleanup(); 
		createDepthResources();
	}

	void DepthBuffer::createDepthResources()
	{
		VkFormat depthFormat = findDepthFormat();
		Texture::createImage(
			m_Device,
			m_Swapchain->getSwapchainExtent().width,
			m_Swapchain->getSwapchainExtent().height,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_DepthImage,
			m_DepthImageMemory);
		m_DepthImageView = m_Swapchain->createImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	//helper
	VkFormat DepthBuffer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_Device->getPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR and (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL and (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	VkFormat DepthBuffer::findDepthFormat()
	{
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool DepthBuffer::hasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
}