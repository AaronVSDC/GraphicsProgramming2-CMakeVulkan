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


	struct GeometryPC
	{
		glm::mat4 transform;       //  64 bytes
		glm::mat4 modelMatrix;     //  64 bytes
		uint32_t albedoIndex;      //   4 bytes
		uint32_t normalIndex;      //   4 bytes
		uint32_t metalRoughIndex;  //   4 bytes
		uint32_t occlusionIndex;   //   4 bytes

	};

	struct ResolutionCameraPush {
		glm::vec2 resolution;
		float      _pad0[2];
		glm::vec3 cameraPos;
		float      _pad1;
	};

	struct DepthPush
	{
		glm::mat4 mvp;
		uint32_t baseColorIndex;
	};

	struct ToneMappingPush {
		float aperture;     // f-stop
		float shutterSpeed;  // in seconds
		float iso;           // sensor sensitivity
	};

	DeferredRenderSystem::DeferredRenderSystem(Device& device, VkExtent2D extent, VkFormat swapFormat)
		:m_Device{ device }
	{
		assert(device.properties.limits.maxPushConstantsSize > sizeof(GeometryPC) && "Max supported push constant data is smaller than 256 bytes");
		Initialize(extent, swapFormat);
	}

	DeferredRenderSystem::~DeferredRenderSystem()
	{
		vkDestroyDescriptorPool(m_Device.device(), m_LightDescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_Device.device(), m_LightDescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.device(), m_LightPipelineLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.device(), m_GeometryPipelineLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.device(), m_DepthPrepassPipelineLayout, nullptr);
		Texture::cleanupBindless(m_Device);
	}

	void DeferredRenderSystem::Initialize(VkExtent2D extent, VkFormat swapFormat)
	{
		m_GBuffer.create(m_Device, extent.width, extent.height);
		m_LightBuffer.create(m_Device, extent.width, extent.height); 

		CreateDepthPrepassPipelineLayout();
		CreateDepthPrepassPipeline();
		CreateGeometryPipelineLayout();
		CreateGeometryPipeline();
		CreateLightingPipelineLayout();
		CreateLightingPipeline();
		CreateLightingDescriptorSet();
		// 4) Blit/tone-map pass reads HDR, writes swapchain
		CreateBlitPipelineLayout();
		CreateBlitPipeline(swapFormat);
		CreateBlitDescriptorSet();
	}

#pragma region DEPTH_PREPASS_PIPELINE

	void DeferredRenderSystem::CreateDepthPrepassPipelineLayout()
	{


		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(DepthPush);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		VkDescriptorSetLayout setLayouts[] = { Texture::s_BindlessSetLayout };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setLayouts;

		if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_DepthPrepassPipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create geometry pipeline layout");
		}

	}

	void DeferredRenderSystem::CreateDepthPrepassPipeline()
	{
		PipelineConfigInfo depthConfig{};
		Pipeline::DefaultPipelineConfigInfo(depthConfig);

		auto bindDescs = Model::Vertex::GetBindingDescriptions();
		auto attrDescs = Model::Vertex::GetAttributeDescriptions();
		depthConfig.vertexBindings = { bindDescs[0] };
		depthConfig.vertexAttributes = { attrDescs[0], attrDescs[3] };
		depthConfig.colorAttachmentFormats.clear();
		depthConfig.renderingInfo.colorAttachmentCount = 0;
		depthConfig.renderingInfo.pColorAttachmentFormats = nullptr;
		depthConfig.colorBlendInfo.attachmentCount = 0;
		depthConfig.colorBlendInfo.pAttachments = nullptr;

		depthConfig.depthAttachmentFormat = GBuffer::DEPTH_FORMAT;
		depthConfig.renderingInfo.depthAttachmentFormat = depthConfig.depthAttachmentFormat;

		depthConfig.pipelineLayout = m_DepthPrepassPipelineLayout;

		m_DepthPrepassPipeline = std::make_unique<Pipeline>(
			m_Device,
			depthConfig,
			"Shaders/DepthPrepass.vert.spv",
			"Shaders/DepthPrepass.frag.spv"
		);
	}

	void DeferredRenderSystem::RenderDepthPrepass(
		VkCommandBuffer commandBuffer,
		std::vector<GameObject>& gameObjects,
		const Camera& camera)
	{
		m_DepthPrepassPipeline->Bind(commandBuffer);
		Texture::bind(commandBuffer, m_DepthPrepassPipelineLayout);


		auto projectionViewMatrix = camera.GetProjectionMatrix() * camera.GetViewMatrix();

		for (auto& gameObject : gameObjects)
		{
			for (auto& sm : gameObject.m_Model->getData().submeshes)
			{
				auto& mat = gameObject.m_Model->getData().materials[sm.materialIndex];
				DepthPush push{};
				auto modelMatrix = gameObject.m_Transform.mat4();
				push.mvp = projectionViewMatrix * modelMatrix;
				push.baseColorIndex = mat.baseColorIndex;


				vkCmdPushConstants(
					commandBuffer,
					m_DepthPrepassPipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(DepthPush),
					&push
				);

				gameObject.m_Model->Bind(commandBuffer);
				gameObject.m_Model->Draw(commandBuffer, sm.indexCount, sm.firstIndex);
			}
		}

	}


#pragma endregion

#pragma region GEOMETRY_PIPELINE
	void DeferredRenderSystem::CreateGeometryPipelineLayout()
	{

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(GeometryPC);

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
			GBuffer::ALBEDO_FORMAT,
			GBuffer::METALROUGH_FORMAT,
			GBuffer::OCCLUSION_FORMAT
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
		cfg.depthStencilInfo.depthTestEnable = VK_TRUE;
		cfg.depthStencilInfo.depthWriteEnable = VK_FALSE;  // <— do not write
		cfg.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;


		cfg.pipelineLayout = m_GeometryPipelineLayout;
		// set cfg.renderingInfo.colorAttachmentCount = 3, pColorAttachmentFormats = cfg.colorAttachmentFormats.data(), depthAttachmentFormat = GBuffer::DEPTH_FORMAT
		m_GeometryPipeline = std::make_unique<Pipeline>(m_Device, cfg,
			"Shaders/GeometryPass.vert.spv",
			"Shaders/GeometryPass.frag.spv");

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
				auto& mat = gameObject.m_Model->getData().materials[sm.materialIndex];
				GeometryPC push{};
				auto modelMatrix = gameObject.m_Transform.mat4();
				GeometryPC pc{};
				push.transform = projectionViewMatrix * modelMatrix;
				push.modelMatrix = modelMatrix;
				push.albedoIndex = mat.baseColorIndex;
				push.normalIndex = mat.normalIndex;
				push.metalRoughIndex = mat.metallicRoughIndex;
				push.occlusionIndex = mat.occlusionIndex;


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
		//update gameobjects here
	}


	void DeferredRenderSystem::RecreateGBuffer(VkExtent2D extent, VkFormat swapFormat)
	{
		vkDeviceWaitIdle(m_Device.device());

		m_GBuffer.cleanup();
		m_GBuffer.create(m_Device, extent.width, extent.height);

		vkDestroyDescriptorPool(m_Device.device(), m_LightDescriptorPool, nullptr);
		CreateLightingDescriptorSet();
	}
#pragma endregion

#pragma region LIGHTING_PIPELINE
	void DeferredRenderSystem::CreateLightingPipelineLayout()
	{
		// one binding of an array-of-5 combined-image-samplers
		VkDescriptorSetLayoutBinding b{};
		b.binding = 0;
		b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		b.descriptorCount = 5;
		b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo dsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		dsInfo.bindingCount = 1;
		dsInfo.pBindings = &b;
		vkCreateDescriptorSetLayout(m_Device.device(), &dsInfo, nullptr, &m_LightDescriptorSetLayout);

		// push const for camera pos and screen texel
		VkPushConstantRange pc{};
		pc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pc.offset = 0;
		pc.size = sizeof(ResolutionCameraPush);

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
		poolSize.descriptorCount = 5;

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

		std::array<VkDescriptorImageInfo, 5> imageInfos = {
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
		  },
			VkDescriptorImageInfo{
				m_GBuffer.getMetalRoughSampler(),
				m_GBuffer.getMetalRoughView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				  },
			VkDescriptorImageInfo{
				m_GBuffer.getOcclusionSampler(),
				m_GBuffer.getOcclusionView(),
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


	void DeferredRenderSystem::CreateLightingPipeline()
	{
		PipelineConfigInfo cfg{};
		Pipeline::DefaultPipelineConfigInfo(cfg);
		cfg.colorAttachmentFormats = { LightBuffer::HDR_FORMAT };
		cfg.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		cfg.pipelineLayout = m_LightPipelineLayout;
		cfg.renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		cfg.renderingInfo.colorAttachmentCount = 1;
		cfg.renderingInfo.pColorAttachmentFormats = cfg.colorAttachmentFormats.data();
		cfg.renderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

		m_LightPipeline = std::make_unique<Pipeline>(
			m_Device, cfg,
			"Shaders/Triangle.vert.spv",
			"Shaders/LightingPass.frag.spv"
		);
	}

	void DeferredRenderSystem::RenderLighting(VkCommandBuffer cb, const Camera& camera, VkExtent2D extent)
	{
		ResolutionCameraPush pushConstantData;
		pushConstantData.resolution = glm::vec2(
			static_cast<float>(extent.width),
			static_cast<float>(extent.height)
		);
		pushConstantData.cameraPos = camera.GetPosition();

		m_LightPipeline->Bind(cb);
		vkCmdBindDescriptorSets(cb,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_LightPipelineLayout, 0, 1, &m_LightDescriptorSet, 0, nullptr);

		vkCmdPushConstants(cb,
			m_LightPipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(pushConstantData),
			&pushConstantData);

		// draw triangle trick
		vkCmdDraw(cb, 3, 1, 0, 0); 
	}


#pragma endregion

#pragma region BLITTING

	void DeferredRenderSystem::CreateBlitPipelineLayout()
	{
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = 0;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo dsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		dsInfo.bindingCount = 1;
		dsInfo.pBindings = &binding;
		if (vkCreateDescriptorSetLayout(m_Device.device(), &dsInfo, nullptr,
			&m_BlitDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create blit descriptor set layout");
		}

		VkPushConstantRange pc{};
		pc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pc.offset = 0;
		pc.size = sizeof(ToneMappingPush);

		VkPipelineLayoutCreateInfo plInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		plInfo.setLayoutCount = 1;
		plInfo.pSetLayouts = &m_BlitDescriptorSetLayout;
		plInfo.pushConstantRangeCount = 1;
		plInfo.pPushConstantRanges = &pc;
		if (vkCreatePipelineLayout(m_Device.device(), &plInfo, nullptr,
			&m_BlitPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create blit pipeline layout");
		}

	}

	void DeferredRenderSystem::CreateBlitPipeline(VkFormat swapFormat)
	{
		PipelineConfigInfo cfg{};
		Pipeline::DefaultPipelineConfigInfo(cfg); 

		// no vertex buffers: triangle uses gl_VertexIndex
		cfg.vertexBindings.clear();
		cfg.vertexAttributes.clear();

		cfg.colorAttachmentFormats = { swapFormat };
		cfg.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		cfg.pipelineLayout = m_BlitPipelineLayout;

		cfg.renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		cfg.renderingInfo.colorAttachmentCount = 1;
		cfg.renderingInfo.pColorAttachmentFormats = cfg.colorAttachmentFormats.data();

		m_BlitPipeline = std::make_unique<Pipeline>(
			m_Device,
			cfg,
			"Shaders/Triangle.vert.spv",
			"Shaders/Blit.frag.spv"
		);
	}

	void DeferredRenderSystem::CreateBlitDescriptorSet()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = 1;
		if (vkCreateDescriptorPool(m_Device.device(), &poolInfo, nullptr,
			&m_BlitDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create blit descriptor pool");
		}

		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = m_BlitDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_BlitDescriptorSetLayout;
		if (vkAllocateDescriptorSets(m_Device.device(), &allocInfo,
			&m_BlitDescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate blit descriptor set");
		}

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = m_LightBuffer.getSampler();
		imageInfo.imageView = m_LightBuffer.getImageView();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		 
		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstSet = m_BlitDescriptorSet;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_Device.device(), 1, &write, 0, nullptr);
	}

	void DeferredRenderSystem::RenderBlit(VkCommandBuffer commandBuffer)
	{

		ToneMappingPush push;
		push.aperture = 2.8f;
		push.shutterSpeed = 1.f / 60.f;
		push.iso = 100.f; 

		// Bind blit/tone-map pipeline
		m_BlitPipeline->Bind(commandBuffer);
		vkCmdBindDescriptorSets(
			commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_BlitPipelineLayout, 0, 1,
			&m_BlitDescriptorSet, 0, nullptr);

		// Push ToneMappingUniforms
		vkCmdPushConstants(
			commandBuffer,
			m_BlitPipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(push),
			&push);

		// Fullscreen triangle
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}

#pragma enregion


}