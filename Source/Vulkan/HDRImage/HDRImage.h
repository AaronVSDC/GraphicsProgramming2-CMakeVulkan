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
	private: 

		void RenderToCubeMap(const VkExtent2D& extent, uint32_t mipLevels, const std::string& vertPath, const std::string& fragPath, VkImage&
			inputImage, const VkImageView& inputImageView, VkSampler
			inputSampler, VkImage& outputCubeMapImage, std::array<std::vector<VkImageView>, 6>& outputCubeMapImageViews);

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
		VkExtent2D m_CubeMapExtent{ 512, 512 };
		std::array<std::vector<VkImageView>, m_FACE_COUNT> m_CubeMapFaceViews;


		// CUBE
		const glm::vec3 m_EYE = glm::vec3(0.0f);
		const glm::mat4 m_CAPTURE_VIEWS[m_FACE_COUNT] =
		{
			glm::lookAt(m_EYE, m_EYE + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // +X
			glm::lookAt(m_EYE, m_EYE + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // -X
			glm::lookAt(m_EYE, m_EYE + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), // -Y
			glm::lookAt(m_EYE, m_EYE + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), // +Y
			glm::lookAt(m_EYE, m_EYE + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // +Z
			glm::lookAt(m_EYE, m_EYE + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) // -Z
		};
		glm::mat4 m_CAPTURE_PROJECTION;

		const std::string m_CubeVertPath = "Shaders/Cube.vert.spv";
		const std::string m_SkyFragPath = "Shaders/Sky.frag.spv";
		const std::string m_IBLFragPath = "shaders/ibl.frag.spv";



	};
}
