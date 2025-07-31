#include "Pipeline.h"
#include "Model.h"
//std
#include <fstream>
#include <iostream>
#include <cassert>
namespace cve
{
	Pipeline::Pipeline(	Device& device,
						const PipelineConfigInfo& configInfo,
						const std::string& vertFilePath,
						const std::string& fragFilePath)
		:m_Device{device}
	{
		CreateGraphicsPipeline(configInfo, vertFilePath, fragFilePath); 
	}
	Pipeline::~Pipeline()
	{
		vkDestroyShaderModule(m_Device.device(), m_VertShaderModule, nullptr); 
		vkDestroyShaderModule(m_Device.device(), m_FragShaderModule, nullptr); 
		vkDestroyPipeline(m_Device.device(), m_GraphicsPipeline, nullptr); 



	}

	std::vector<char> Pipeline::readFile(const std::string& filePath)
	{
		std::ifstream file{ filePath, std::ios::ate | std::ios::binary }; 

		if (!file.is_open())
		{
			throw std::runtime_error("(look inside \"readfile\" function inside pipeline class) failed to open file: " + filePath);
		}

		size_t fileSize = static_cast<size_t>(file.tellg()); 
		std::vector<char> buffer(fileSize); 

		file.seekg(0); 
		file.read(buffer.data(), fileSize); 

		file.close(); 
		return buffer; 


	}
	void Pipeline::CreateGraphicsPipeline(const PipelineConfigInfo& configInfo,
		const std::string& vertFilePath,
		const std::string& fragFilePath)
	{
		assert(configInfo.pipelineLayout != VK_NULL_HANDLE && 
			"Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
		assert(configInfo.renderPass != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline: no renderPass provided in configInfo");

		std::vector<char> vertCode = readFile(vertFilePath); 
		std::vector<char> fragCode = readFile(fragFilePath); 

		CreateShaderModule(vertCode, &m_VertShaderModule);
		CreateShaderModule(fragCode, &m_FragShaderModule);

		const uint8_t amountOfShaders = 2; 
		//similar to Direct3D you have to set shaderStages and specify a bunch of stuff like the kind of shader (vertex, fragment,...)
		VkPipelineShaderStageCreateInfo shaderStages[amountOfShaders]; 

		//vertex shader
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; 
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT; 
		shaderStages[0].module = m_VertShaderModule; 
		shaderStages[0].pName = "main"; //name to the entry function of the shader (I guess it can be changed then if you change both the function and this name?)
		shaderStages[0].flags = 0; 
		shaderStages[0].pNext = nullptr; 
		shaderStages[0].pSpecializationInfo = nullptr; 

		//fragment shader
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = m_FragShaderModule;
		shaderStages[1].pName = "main"; //name to the entry function of the shader (I guess it can be changed then if you change both the function and this name?)
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;


		auto bindingDescriptions = Model::Vertex::GetBindingDescriptions(); 
		auto attributeDescriptions = Model::Vertex::GetAttributeDescriptions();
		//this struct describes how we interpret the vertexbuffer data (initial input) into the graphics pipeline
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{}; 
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; 
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());  
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); 
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data(); 

		//and about 5 fucking years later create the actual GraphicsPipeline object that uses all the shit that's just initialised
		VkGraphicsPipelineCreateInfo pipelineInfo{}; 
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; 
		pipelineInfo.stageCount = amountOfShaders; 
		pipelineInfo.pStages = shaderStages; 
		//wire the configInfo (selfmade) struct to the pipelineInfo 
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo; 
		pipelineInfo.pViewportState = &configInfo.viewportInfo; 
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo; 
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo; 
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo; 
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo; 
		pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo; 

		pipelineInfo.layout = configInfo.pipelineLayout; 
		pipelineInfo.renderPass = configInfo.renderPass; 
		pipelineInfo.subpass = configInfo.subpass; 

		//Apparently its sometimes more optimized if the gpu uses multiple pipelines, if you want to enable you have to derive from other pipelines and such. Its disabled now but its something over here
		pipelineInfo.basePipelineIndex = -1; 
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; 

		//and finally create the shit
		if (vkCreateGraphicsPipelines(m_Device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline"); 
		}

	}
	void Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(m_Device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module (pipeline class)"); 
		}


	}

	void Pipeline::DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
	{

		//----------------------------------------------------------------------
		//INPUT ASSEMBLY STAGE
		//----------------------------------------------------------------------
	
		//Triangle input type (strip, list, etc) 
		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; 
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; 
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE; 

		configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO; 
		configInfo.viewportInfo.viewportCount = 1; 
		configInfo.viewportInfo.pViewports = nullptr; 
		configInfo.viewportInfo.scissorCount = 1; 
		configInfo.viewportInfo.pScissors = nullptr; 


		//----------------------------------------------------------------------
		//RASTERIZATION STAGE
		//----------------------------------------------------------------------

		//info variable again (VkPipelineRasterizationStateCreateInfo)
		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO; 
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE; //clamps the depthbuffer to either 0 or 1 when exceeding. (kinda stupid i think? because nothing can be behind the screen right?)
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL; //for specifying which type of triangle (filled in, only edges, etc..)
		configInfo.rasterizationInfo.lineWidth = 1.0f; 
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT; //TODO: VERY IMPORTANT FOR PERFORMANCE, WHICH TRIAGLE FACE DO YOU WANT TO DISCARD (CULL)
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; 
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE; 
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.f; //optional
		configInfo.rasterizationInfo.depthBiasClamp = 0.f; //optional
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.f; //optional

		//multisampling (antialiasing and shit) 
		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; 
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE; 
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.f; //optional
		configInfo.multisampleInfo.pSampleMask = nullptr; //optional
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; //optional
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE; //optional

		//colorblending
		//TODO: if you want transparancy, fuck around with this apparently (dont take my word for it though)
		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT; 
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE; 
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; //Optional
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; //Optional
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; //Optional
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; //Optional
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; //Optional
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; //Optional

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; 
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE; 
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; //Optional
		configInfo.colorBlendInfo.attachmentCount = 1; 
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment; 
		configInfo.colorBlendInfo.blendConstants[0] = 0.f; //Optional
		configInfo.colorBlendInfo.blendConstants[1] = 0.f; //Optional
		configInfo.colorBlendInfo.blendConstants[2] = 0.f; //Optional
		configInfo.colorBlendInfo.blendConstants[3] = 0.f; //Optional

		//depth buffer (to render the right triangle ofc so that the closest one is the one rendered) 
		
		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; 
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE; 
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE; 
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE; 
		configInfo.depthStencilInfo.minDepthBounds = 0.f; //optional
		configInfo.depthStencilInfo.maxDepthBounds = 1.f; //optional
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE; 
		configInfo.depthStencilInfo.front = {}; //optional
		configInfo.depthStencilInfo.back = {}; //optional
		 
		configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR }; 
		configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO; 
		configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data(); 
		configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size()); 
		configInfo.dynamicStateInfo.flags = 0; 
	}
	void Pipeline::Bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline); 


	}
}