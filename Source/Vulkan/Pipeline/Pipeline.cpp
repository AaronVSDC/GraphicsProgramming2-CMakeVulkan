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
						std::optional<const std::string> fragFilePath)
		:m_Device{device}
	{
		CreateGraphicsPipeline(configInfo, vertFilePath, fragFilePath); 
	}
	Pipeline::~Pipeline()
	{
		vkDestroyShaderModule(m_Device.device(), m_VertShaderModule, nullptr); 
		if (m_HasFragShader)
		{
			vkDestroyShaderModule(m_Device.device(), m_FragShaderModule, nullptr);
		}
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
		std::optional<const std::string> fragFilePath)
	{
		assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline: no pipelineLayout provided in configInfo");


		std::vector<char> vertCode = readFile(vertFilePath);
		CreateShaderModule(vertCode, &m_VertShaderModule); 

		if (fragFilePath)
		{
			std::vector<char> fragCode = readFile(*fragFilePath);
			CreateShaderModule(fragCode, &m_FragShaderModule);
			m_HasFragShader = true; 
		}
		
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages; 

		// vertex
		VkPipelineShaderStageCreateInfo vertStage{};


		//vertex shader
		vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; 
		vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT; 
		vertStage.module = m_VertShaderModule; 
		vertStage.pName = "main"; //name to the entry function of the shader (I guess it can be changed then if you change both the function and this name?)
		vertStage.flags = 0; 
		vertStage.pNext = nullptr; 
		vertStage.pSpecializationInfo = nullptr;

		shaderStages.push_back(vertStage);

		if (m_HasFragShader)
		{
			VkPipelineShaderStageCreateInfo fragStage{}; 
			//fragment shader
			fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragStage.module = m_FragShaderModule;
			fragStage.pName = "main"; //name to the entry function of the shader (I guess it can be changed then if you change both the function and this name?)
			fragStage.flags = 0;
			fragStage.pNext = nullptr;
			fragStage.pSpecializationInfo = nullptr;
			shaderStages.push_back(fragStage); 
		}


		//this struct describes how we interpret the vertexbuffer data (initial input) into the graphics pipeline
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configInfo.attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(configInfo.bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = configInfo.attributeDescriptions.empty() ? nullptr : configInfo.attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = configInfo.bindingDescriptions.empty() ? nullptr : configInfo.bindingDescriptions.data();

		//and about 5 fucking years later create the actual GraphicsPipeline object that uses all the shit that's just initialised
		VkGraphicsPipelineCreateInfo pipelineInfo{}; 
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; 
		pipelineInfo.stageCount = shaderStages.size(); 
		pipelineInfo.pStages = shaderStages.data(); 
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
		VkPipelineRenderingCreateInfo renderingInfo = configInfo.renderingInfo;
		renderingInfo.colorAttachmentCount = static_cast<uint32_t>(configInfo.colorAttachmentFormats.size());
		renderingInfo.pColorAttachmentFormats = configInfo.colorAttachmentFormats.data();
		renderingInfo.depthAttachmentFormat = configInfo.depthAttachmentFormat;
		pipelineInfo.pNext = &renderingInfo;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;

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
		configInfo.bindingDescriptions = Model::Vertex::GetBindingDescriptions();
		configInfo.attributeDescriptions = Model::Vertex::GetAttributeDescriptions();
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

		configInfo.renderingInfo = {};
		configInfo.renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		configInfo.renderingInfo.colorAttachmentCount = 1;
		configInfo.renderingInfo.pColorAttachmentFormats = nullptr;
		configInfo.renderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		configInfo.renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	}
	void Pipeline::Bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline); 


	}
}
