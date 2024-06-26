#include "SimpleRenderSystem.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm\gtc\constants.hpp>

//std
#include <stdexcept>
#include <array>
#include <iostream>
namespace cve {


	struct SimplePushConstantData
	{
		glm::mat4 transform{ 1.f };
		glm::mat4 modelMatrix{ 1.f }; 
	};

	SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass renderPass) : m_Device{device}
	{
		CreatePipelineLayout();
		CreatePipeline(renderPass);
	}
	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr);
	}

	void SimpleRenderSystem::CreatePipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr; 	// is used to pass vertex data other than vertex and fragment shaders
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}
	void SimpleRenderSystem::CreatePipeline(VkRenderPass renderPass)
	{
		assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_PipelineLayout;
		m_pPipeline = std::make_unique<Pipeline>(
			m_Device,
			pipelineConfig,
			"Shaders/SimpleShader.vert.spv",
			"Shaders/SimpleShader.frag.spv");

	}
	void SimpleRenderSystem::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera)
	{

		m_pPipeline->Bind(commandBuffer);

		auto projectionViewMatrix = camera.GetProjectionMatrix() * camera.GetViewMatrix(); 

		for (auto& gameObject : gameObjects)
		{

			SimplePushConstantData push{};
			auto modelMatrix = gameObject.m_Transform.mat4(); 
			push.transform = projectionViewMatrix * modelMatrix;
			push.modelMatrix = modelMatrix;

			vkCmdPushConstants(commandBuffer,
				m_PipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData),
				&push);

			gameObject.m_Model->Bind(commandBuffer);
			gameObject.m_Model->Draw(commandBuffer);
		}

	}
}