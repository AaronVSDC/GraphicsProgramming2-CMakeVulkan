#pragma once 
#include "Device.h"
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include "glm/vec3.hpp"
#include <memory>
#include <array>

namespace cve
{
	class HDRImage final
	{
	public: 
		HDRImage(Device& device, const std::string& filename);
		~HDRImage();


		HDRImage(const HDRImage&) = delete;
		HDRImage& operator=(const HDRImage&) = delete;
		HDRImage(HDRImage&&) = delete;
		HDRImage& operator=(HDRImage&&) = delete;


		VkImageView& GetCubeMapView() { return m_CubeMapImageView;  }
		VkSampler& GetCubeMapSampler() { return m_CubeMapSampler;  }
		VkImageView& GetIrradianceView() { return m_IrradianceMapImageView; }
		VkSampler& GetIrradianceSampler() { return m_IrradianceMapSampler; }

	private: 

		void RenderToCubeMap(const VkExtent2D& extent, uint32_t mipLevels, const std::string& vertPath, const std::string& fragPath, VkImage&
			inputImage, const VkImageView& inputImageView, VkSampler
			inputSampler, VkImage& outputCubeMapImage, std::array<std::vector<VkImageView>, 6>& outputCubeMapImageViews);

		void CreateIrradianceMap();

		void TransitionImageLayout(VkImage image, VkFormat, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		VkImageAspectFlags GetImageAspect(VkFormat format);
		void CreateCubeMap();
		void CreateEquirectImage(uint32_t width, uint32_t height, uint32_t miplevels, VkFormat format, VkImageUsageFlags usage);
		void CreateEquirectTextureImageView();
		void CreateEquirectTextureSampler(VkFilter filter, VkSamplerAddressMode addressMode);



		Device& m_Device;
		static constexpr int m_FACE_COUNT = 6;

		// IMAGES
		VkImage m_EquirectImage;
		VkDeviceMemory m_EquirectImageMemory;
		VkImageView m_EquirectImageView;
		VkSampler m_EquirectSampler = VK_NULL_HANDLE;
		uint32_t m_EquirectMipLevels{};
		VkFormat m_EquirectFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D m_EquirectExtent{ 0, 0 };
		VkImageLayout m_EquirectImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

		VkImage m_CubeMapImage;
		VkDeviceMemory m_CubeMapImageMemory;
		VkImageView m_CubeMapImageView;
		VkSampler m_CubeMapSampler = VK_NULL_HANDLE;
		VkExtent2D m_CubeMapExtent{ 1024, 1024 };
		std::array<std::vector<VkImageView>, m_FACE_COUNT> m_CubeMapFaceViews;

		VkImage m_IrradianceMapImage;
		VkDeviceMemory m_IrradianceMapImageMemory;
		VkImageView m_IrradianceMapImageView;
		VkSampler m_IrradianceMapSampler = VK_NULL_HANDLE;
		VkExtent2D m_IrradianceMapExtent{ 32, 32 };
		std::array<VkImageView, m_FACE_COUNT> m_IrradianceMapFaceViews;

		const std::string m_CubeVertPath = "Shaders/Cube.vert.spv";
		const std::string m_SkyFragPath = "Shaders/Sky.frag.spv";
		const std::string m_IBLFragPath = "shaders/ImageBasedLighting.frag.spv";



	};
}
