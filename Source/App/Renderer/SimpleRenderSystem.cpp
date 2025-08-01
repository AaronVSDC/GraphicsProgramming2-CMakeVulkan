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
		glm::mat4 transform{ 1.f }; //64B
		glm::mat4 modelMatrix{ 1.f }; //64B
		uint32_t materialIndex; //4B

		//pad out to 16 byte multiple
		uint8_t _pad[12]; 
	};

	SimpleRenderSystem::SimpleRenderSystem(Device& device, VkFormat swapChainImageFormat, VkFormat depthFormat, const std::vector<GameObject>& gameObjects)
	: m_Device{ device }
	{
		std::cout << "max push constant size" << device.properties.limits.maxPushConstantsSize << std::endl;
		assert(device.properties.limits.maxPushConstantsSize > sizeof(SimplePushConstantData)); 
		CreatePipelineLayout();
		CreatePipeline(swapChainImageFormat, depthFormat);
	}
	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr);
		Texture::cleanupBindless(m_Device);
	}

	void SimpleRenderSystem::CreatePipelineLayout()
	{

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkDescriptorSetLayout setLayouts[] = {
			Texture::s_BindlessSetLayout
		};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setLayouts;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}
	void SimpleRenderSystem::CreatePipeline(VkFormat colorFormat, VkFormat depthFormat)
	{
		assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = VK_NULL_HANDLE;
		pipelineConfig.colorAttachmentFormats = { colorFormat };
		pipelineConfig.depthAttachmentFormat = depthFormat;
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
		Texture::bind(commandBuffer, m_PipelineLayout); 


		auto projectionViewMatrix = camera.GetProjectionMatrix() * camera.GetViewMatrix(); 

		for (auto& gameObject : gameObjects)
		{
			for (auto& sm : gameObject.m_Model->getData().submeshes)
			{
				SimplePushConstantData push{};
				auto modelMatrix = gameObject.m_Transform.mat4(); 
				push.transform = projectionViewMatrix * modelMatrix;
				push.modelMatrix = modelMatrix;
				push.materialIndex = sm.materialIndex; 

				vkCmdPushConstants(
					commandBuffer,
					m_PipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(push),
					&push
				);

				gameObject.m_Model->Bind(commandBuffer);
				gameObject.m_Model->Draw(commandBuffer, sm.indexCount, sm.firstIndex); 
			}
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