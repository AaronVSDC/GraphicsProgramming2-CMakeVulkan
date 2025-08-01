#pragma once

#include "Device.h"
#include <string>
#include <vector>
namespace cve {


struct PipelineConfigInfo 
{
	PipelineConfigInfo() = default; 
	PipelineConfigInfo(const PipelineConfigInfo&) = delete; 
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete; 

	VkPipelineViewportStateCreateInfo viewportInfo; 
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector<VkDynamicState> dynamicStateEnables; 
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;

	std::vector<VkFormat> colorAttachmentFormats{};
	VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
	VkPipelineRenderingCreateInfo renderingInfo{};
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
	Pipeline() = default; 

	static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

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


