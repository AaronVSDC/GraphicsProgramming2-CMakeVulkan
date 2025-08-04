#include "DeferredRenderSystem.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm\gtc\constants.hpp>

//std
#include <array>
#include <stdexcept>
#include <iostream>
namespace cve {


	struct SimplePushConstantData
	{
		glm::mat4 transform{ 1.f }; //64B
		glm::mat4 modelMatrix{ 1.f }; //64B
		uint32_t materialIndex; //4B

		uint8_t _pad[12]; 
	};

	struct PCResolutionCamera {
		glm::vec2 resolution;
		float      _pad0[2];
		glm::vec3 cameraPos;
		float      _pad1;
	};
	static_assert(sizeof(PCResolutionCamera) == 32, "Push-constant struct size mismatch");

	DeferredRenderSystem::DeferredRenderSystem(Device& device, VkExtent2D extent, VkFormat swapFormat)
		:m_Device{device}
	{
		assert(device.properties.limits.maxPushConstantsSize > sizeof(SimplePushConstantData) && "Max supported push constant data is smaller than 256 bytes");
		Initialize(extent, swapFormat); 
	}

	DeferredRenderSystem::~DeferredRenderSystem()
	{
		vkDestroyDescriptorPool(m_Device.device(), m_LightDescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_Device.device(), m_LightDescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.device(), m_LightPipelineLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.device(), m_GeometryPipelineLayout, nullptr);
		Texture::cleanupBindless(m_Device);
	}

	void DeferredRenderSystem::Initialize(VkExtent2D extent, VkFormat swapFormat)
	{
		m_GBuffer.create(m_Device, extent.width, extent.height);
		CreateGeometryPipelineLayout();
		CreateGeometryPipeline();
		CreateLightingPipelineLayout();
		CreateLightingPipeline(swapFormat);
		CreateLightingDescriptorSet(); 
	}
	void DeferredRenderSystem::CreateGeometryPipelineLayout()
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

		if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_GeometryPipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create geometry pipeline layout");
		}
	}
	void DeferredRenderSystem::CreateGeometryPipeline()
	{
		assert(m_GeometryPipelineLayout != nullptr && "Cannot create geometry pipeline before geometry pipeline layout"); 

		PipelineConfigInfo cfg{};
		Pipeline::DefaultPipelineConfigInfo(cfg);
		cfg.vertexBindings = Model::Vertex::GetBindingDescriptions();
		cfg.vertexAttributes = Model::Vertex::GetAttributeDescriptions(); 
		cfg.colorAttachmentFormats = {
		  GBuffer::POS_FORMAT,
		  GBuffer::NORM_FORMAT,
		  GBuffer::ALBEDO_FORMAT
		};
		cfg.depthAttachmentFormat = GBuffer::DEPTH_FORMAT;
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(
			cfg.colorAttachmentFormats.size(),
			cfg.colorBlendAttachment  
		);
		cfg.colorBlendInfo.attachmentCount =
			static_cast<uint32_t>(blendAttachments.size());
		cfg.colorBlendInfo.pAttachments = blendAttachments.data();

		cfg.renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		cfg.renderingInfo.colorAttachmentCount = static_cast<uint32_t>(cfg.colorAttachmentFormats.size());
		cfg.renderingInfo.pColorAttachmentFormats = cfg.colorAttachmentFormats.data();
		cfg.renderingInfo.depthAttachmentFormat = cfg.depthAttachmentFormat;

		cfg.pipelineLayout = m_GeometryPipelineLayout;
		// set cfg.renderingInfo.colorAttachmentCount = 3, pColorAttachmentFormats = cfg.colorAttachmentFormats.data(), depthAttachmentFormat = GBuffer::DEPTH_FORMAT
		m_GeometryPipeline = std::make_unique<Pipeline>(m_Device, cfg,
			"Shaders/GeometryPass.vert.spv",
			"Shaders/GeometryPass.frag.spv");

	}

	void DeferredRenderSystem::CreateLightingPipelineLayout()
	{
		// one binding of an array-of-3 combined-image-samplers
		VkDescriptorSetLayoutBinding b{};
		b.binding = 0;
		b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		b.descriptorCount = 3;
		b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo dsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		dsInfo.bindingCount = 1;
		dsInfo.pBindings = &b;
		vkCreateDescriptorSetLayout(m_Device.device(), &dsInfo, nullptr, &m_LightDescriptorSetLayout);

		// push const for camera pos and screen texel
		VkPushConstantRange pc{};
		pc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pc.offset = 0;
		pc.size = sizeof(PCResolutionCamera);  

		VkPipelineLayoutCreateInfo plInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		plInfo.setLayoutCount = 1;
		plInfo.pSetLayouts = &m_LightDescriptorSetLayout;
		plInfo.pushConstantRangeCount = 1;
		plInfo.pPushConstantRanges = &pc;
		vkCreatePipelineLayout(m_Device.device(), &plInfo, nullptr, &m_LightPipelineLayout);
	}
	void DeferredRenderSystem::CreateLightingDescriptorSet()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = 3;

		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = 1;
		if (vkCreateDescriptorPool(m_Device.device(), &poolInfo, nullptr, &m_LightDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create lighting descriptor pool");
		}

		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = m_LightDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_LightDescriptorSetLayout;
		if (vkAllocateDescriptorSets(m_Device.device(), &allocInfo, &m_LightDescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate lighting descriptor set");
		}

		std::array<VkDescriptorImageInfo, 3> imageInfos = {
		  VkDescriptorImageInfo{
			m_GBuffer.getPositionSampler(),
			m_GBuffer.getPositionView(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		  },
		  VkDescriptorImageInfo{
			m_GBuffer.getNormalSampler(),
			m_GBuffer.getNormalView(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		  },
		  VkDescriptorImageInfo{
			m_GBuffer.getAlbedoSpecSampler(),
			m_GBuffer.getAlbedoSpecView(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		  }
		};

		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstSet = m_LightDescriptorSet;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorCount = static_cast<uint32_t>(imageInfos.size());
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = imageInfos.data();

		vkUpdateDescriptorSets(m_Device.device(), 1, &write, 0, nullptr);
	}

	void DeferredRenderSystem::CreateLightingPipeline(VkFormat swapFormat)
	{
		PipelineConfigInfo cfg{};
		Pipeline::DefaultPipelineConfigInfo(cfg);
		cfg.vertexBindings.clear();
		cfg.vertexAttributes.clear(); 

		cfg.colorAttachmentFormats = { swapFormat };
		cfg.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		cfg.pipelineLayout = m_LightPipelineLayout;
		cfg.renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		cfg.renderingInfo.colorAttachmentCount = 1;
		cfg.renderingInfo.pColorAttachmentFormats = cfg.colorAttachmentFormats.data();
		cfg.renderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

		m_LightPipeline = std::make_unique<Pipeline>(
			m_Device, cfg,
			"Shaders/FullScreenPass.vert.spv",
			"Shaders/FullScreenPass.frag.spv"
		);
	}


	void DeferredRenderSystem::RenderGeometry(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera)
	{

		m_GeometryPipeline->Bind(commandBuffer);
		Texture::bind(commandBuffer, m_GeometryPipelineLayout);  


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
					m_GeometryPipelineLayout, 
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

	void DeferredRenderSystem::UpdateGeometry(std::vector<GameObject>& gameObjects, float deltaTime)
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

	void DeferredRenderSystem::RenderLighting(VkCommandBuffer cb, const Camera& camera, VkExtent2D extent)
	{

		PCResolutionCamera pushConstantData;

		pushConstantData.resolution = glm::vec2(
			static_cast<float>(extent.width),
			static_cast<float>(extent.height)
		);
		pushConstantData.cameraPos = camera.GetPosition(); 

		m_LightPipeline->Bind(cb);
		vkCmdBindDescriptorSets(cb,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_LightPipelineLayout, 0, 1, &m_LightDescriptorSet, 0, nullptr);
		// push camera position for lighting calculations
		vkCmdPushConstants(cb,
			m_LightPipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(pushConstantData),
			&pushConstantData);
		// draw fullscreen quad
		vkCmdDraw(cb,    // no vertex buffer bound 
			3,               // exactly 4 vertices
			1,               // one instance
			0,               // firstVertex = 0
			0                // firstInstance = 0
		);
	}

	void DeferredRenderSystem::RecreateGBuffer(VkExtent2D extent, VkFormat swapFormat)
	{
		vkDeviceWaitIdle(m_Device.device()); 

		m_GBuffer.cleanup();
		m_GBuffer.create(m_Device, extent.width, extent.height);

		vkDestroyDescriptorPool(m_Device.device(), m_LightDescriptorPool, nullptr);
		CreateLightingDescriptorSet();
	}

}