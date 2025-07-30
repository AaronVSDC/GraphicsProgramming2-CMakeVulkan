#pragma once
#include "Buffer.hpp"
#include "../Core/Device.hpp"
#include "../Core/Swapchain.hpp"

namespace cvr
{
	class DepthBuffer final
	{
	public:
		DepthBuffer(Device* device, Swapchain* swapchain);
		~DepthBuffer() = default; 

		DepthBuffer(const DepthBuffer&) = delete;
		DepthBuffer(DepthBuffer&&) = delete;
		DepthBuffer& operator=(const DepthBuffer&) = delete;
		DepthBuffer& operator=(DepthBuffer&&) = delete;


		VkImageView& getDepthImageView() { return m_DepthImageView;  }

	private:
		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;

		void createDepthResources();
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat();
		bool hasStencilComponent(VkFormat format);


		//ref
		Swapchain* m_Swapchain;
		Device* m_Device; 


	};



}