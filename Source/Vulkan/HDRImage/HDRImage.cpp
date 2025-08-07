#include "HDRImage.h"
#include <array>
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include "Pipeline.h"
#include <memory>

namespace cve
{
	HDRImage::HDRImage(Device& device, const std::string& filename)
		:m_Device{device}
	{
	}
	HDRImage::~HDRImage()
	{
	}
	void HDRImage::RenderToCubeMap(const VkExtent2D& extent,
		uint32_t mipLevels,
		const std::string& vertPath,
		const std::string& fragPath,
		VkImage& inputImage,
		const VkImageView& inputImageView,
		VkSampler inputSampler,
		VkImage& outputCubeMapImage,
		std::array<std::vector<VkImageView>, 6>& outputCubeMapImageViews)
	{

	}
}