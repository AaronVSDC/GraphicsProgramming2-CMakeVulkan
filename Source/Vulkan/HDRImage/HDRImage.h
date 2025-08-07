#pragma once 
#include "Device.h" 
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

	private: 

		void RenderToCubeMap(const VkExtent2D& extent, uint32_t mipLevels, const std::string& vertPath, const std::string& fragPath, VkImage&
			inputImage, const VkImageView& inputImageView, VkSampler
			inputSampler, VkImage& outputCubeMapImage, std::array<std::vector<VkImageView>, 6>& outputCubeMapImageViews);



	};
}