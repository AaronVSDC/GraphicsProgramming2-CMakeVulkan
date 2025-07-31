#include "SimpleRenderSystem.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm\gtc\constants.hpp>

//std
#include <stdexcept>
#include <iostream>
namespace cve {


	struct SimplePushConstantData
	{
		glm::mat4 transform{ 1.f };
		glm::mat4 modelMatrix{ 1.f }; 
	};

	SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass renderPass, const std::vector<GameObject>& gameObjects) : m_Device{device}
	{
		Texture::initDescriptors(device, gameObjects.size());

		CreatePipelineLayout();
		CreatePipeline(renderPass);

		for (auto& gameObject : gameObjects)
		{
			gameObject.m_Texture->allocateDescriptorSet(); 
		}
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
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &Texture::getDescriptorSetLayout();
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

		for (size_t i = 0; i < gameObjects.size(); ++i)
		{
			SimplePushConstantData push{};
			auto modelMatrix = gameObjects[i].m_Transform.mat4();
			push.transform = projectionViewMatrix * modelMatrix;
			push.modelMatrix = modelMatrix;

			vkCmdPushConstants(commandBuffer,
				m_PipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData),
				&push);

			gameObjects[i].m_Texture->bind(commandBuffer, m_PipelineLayout); 
			gameObjects[i].m_Model->Bind(commandBuffer);
			gameObjects[i].m_Model->Draw(commandBuffer);
		}
	}

	void SimpleRenderSystem::UpdateGameObjects(std::vector<GameObject>& gameObjects, float deltaTime)
	{
		//constexpr GameObject::id_t kVikingRoomID = 0;
		//constexpr float degPerSec = 30.f;
		//constexpr float radPerSec = glm::radians(degPerSec);

		//for (auto& obj : gameObjects)
		//{
		//	if (obj.GetID() == kVikingRoomID) {
		//		obj.m_Transform.rotation.y += radPerSec * deltaTime;
		//	}
		//}

	}

}