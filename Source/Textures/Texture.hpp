#pragma once
#include <vulkan/vulkan.h>
#include "../Buffers/Buffer.hpp"
#include "../Core/Device.hpp"
#include "../Core/Swapchain.hpp"

namespace cvr
{
	class Texture final : public Buffer
	{
	public:

		Texture(Device* device, Swapchain* swapchain);
		~Texture();


		VkSampler getTextureSampler() const { return m_TextureSampler;  }
		VkImageView getTextureImageView() const { return m_TextureImageView;  }

	private: 

		VkImage m_TextureImage;
		VkDeviceMemory m_TextureImageMemory;
		VkImageView m_TextureImageView;
		VkSampler m_TextureSampler;
		Swapchain* m_Swapchain;
		std::string TEXTURE_PATH = "Textures/viking_room.png";

		void createTextureImage();
		void createImage(uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory);
		void transitionImageLayout(VkImage image,
			VkFormat format,
			VkImageLayout oldLayout,
			VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer,
			VkImage image,
			uint32_t width,
			uint32_t height);
		void createTextureImageView(); 
		void createTextureSampler();

	};

}