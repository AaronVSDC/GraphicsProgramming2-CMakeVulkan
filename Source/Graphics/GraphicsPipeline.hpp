#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <string>
#include "../Core/Device.hpp"
#include "../Core/Swapchain.hpp"
#include "../Graphics/RenderPass.hpp"
#include "../Graphics/Descriptors.hpp"

namespace cvr
{
	class GraphicsPipeline final
	{
	public:

		GraphicsPipeline(Device* device, Swapchain* swapchain, DescriptorManager* descriptors, RenderPass* renderPass); 
		~GraphicsPipeline(); 

		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline(const GraphicsPipeline&&) = delete; 
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete; 
		GraphicsPipeline& operator=(const GraphicsPipeline&&) = delete; 


		VkPipeline& getGraphicsPipeline() { return m_GraphicsPipeline;  }
		VkPipelineLayout& getPipelineLayout() { return m_PipelineLayout;  }

	private: 

		void createGraphicsPipeline(); 
		static std::vector<char> readFile(const std::string& filename);
		VkShaderModule createShaderModule(const std::vector<char>& code) const;

		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;

		//ref
		Swapchain* m_Swapchain;
		Device* m_Device;
		DescriptorManager* m_Descriptors;
		RenderPass* m_RenderPass; 

	};


}


