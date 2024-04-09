#pragma once

#include "Device.h"
#include <string>
#include <vector>
namespace cve {


struct PipelineConfigInfo 
{
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class Pipeline
{
public: 

	Pipeline(Device& device, 
			 const PipelineConfigInfo& configInfo,
			 const std::string& vertFilePath ,
			 const std::string& fragFilePath); 

	~Pipeline();

	Pipeline(const Pipeline& other) = delete; 
	Pipeline& operator=(const Pipeline& rhs) = delete; 

	static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t width, uint32_t height);

	void Bind(VkCommandBuffer commandBuffer); 


private: 

	static std::vector<char> readFile(const std::string& filePath); 
	void CreateGraphicsPipeline(const PipelineConfigInfo& configInfo, 
								const std::string& verFilePath, 
								const std::string& fragFilePath); 
	void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule); 

	Device& m_Device; //agregation, be carefull not to create a dangling pointer
	VkPipeline m_GraphicsPipeline; 
	VkShaderModule m_VertShaderModule; 
	VkShaderModule m_FragShaderModule; 


};
}


