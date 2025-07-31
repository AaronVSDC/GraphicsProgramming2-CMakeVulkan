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

	SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass renderPass, const std::vector<std::unique_ptr<Texture>>& textures) : m_Device{device}
	{
		CreatePipelineLayout();
		CreatePipeline(renderPass);
		std::vector<Texture*> texPtrs;
		texPtrs.reserve(textures.size());
		for (auto& uptr : textures) {
			texPtrs.push_back(uptr.get());
		}
		SetupDescriptorSets(texPtrs);
	}
	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr);
		vkDestroyDescriptorPool(m_Device.device(), m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_Device.device(), m_DescriptorSetLayout, nullptr);
		
	}

	void SimpleRenderSystem::CreatePipelineLayout()
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &samplerLayoutBinding;

		if (vkCreateDescriptorSetLayout(
			m_Device.device(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
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
	void SimpleRenderSystem::SetupDescriptorSets(const std::vector<Texture*>& textures)
	{
		//TODO: CHANGE TO ACCOUNT FOR HOW MANY GAME OBJECTS ARE IN SCENE
		size_t objectCount = 1; 

		// 3a: descriptor pool
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = static_cast<uint32_t>(objectCount);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(objectCount);

		if (vkCreateDescriptorPool(
			m_Device.device(),
			&poolInfo,
			nullptr,
			&m_DescriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool");
		}

		// 3b: allocate one descriptor set per object
		std::vector<VkDescriptorSetLayout> layouts(objectCount, m_DescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(objectCount);
		allocInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(objectCount);
		if (vkAllocateDescriptorSets(
			m_Device.device(),
			&allocInfo,
			m_DescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets");
		}

		// 3c: write each descriptor set to point at its Texture
		for (size_t i = 0; i < objectCount; i++) {
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textures[i]->getImageView();  // your Texture class instance
			imageInfo.sampler = textures[i]->getSampler();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = 0;  // must match binding in shader
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(
				m_Device.device(),
				1, &descriptorWrite,
				0, nullptr);
		}
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
			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_PipelineLayout,
				0,              // first set
				1, &m_DescriptorSets[i],
				0, nullptr);


			gameObjects[i].m_Model->Bind(commandBuffer);
			gameObjects[i].m_Model->Draw(commandBuffer);
		}
	}

	void SimpleRenderSystem::UpdateGameObjects(std::vector<GameObject>& gameObjects, float deltaTime)
	{
		constexpr GameObject::id_t kVikingRoomID = 0;
		constexpr float degPerSec = 30.f;
		constexpr float radPerSec = glm::radians(degPerSec);

		for (auto& obj : gameObjects)
		{
			if (obj.GetID() == kVikingRoomID) {
				obj.m_Transform.rotation.y += radPerSec * deltaTime;
			}
		}

	}

}