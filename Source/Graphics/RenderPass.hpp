#pragma once
#include <vulkan/vulkan.h>
#include "../Core/Swapchain.hpp"
#include "../Core/Device.hpp"
namespace cvr
{
	class RenderPass
	{
	public:
		RenderPass(Swapchain* swapchain, Device* device);
		~RenderPass();

		RenderPass(const RenderPass&) = delete;
		RenderPass(const RenderPass&&) = delete;
		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass& operator=(const RenderPass&&) = delete;


		VkRenderPass getRenderPass() const { return m_RenderPass;  }


	private:


		void createRenderPass();


		VkRenderPass m_RenderPass;

		//ref
		Swapchain* m_Swapchain; 
		Device* m_Device;


		//TEMPORARY HELPERS:
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat();

	};


}
