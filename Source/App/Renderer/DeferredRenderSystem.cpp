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



	DeferredRenderSystem::DeferredRenderSystem(Device& device, VkExtent2D extent, VkFormat swapFormat, std::vector<PointLight>& lights)
		:m_Device{ device }, m_CPULights{lights}
	{
		assert(device.properties.limits.maxPushConstantsSize > sizeof(GeometryPC) && "Max supported push constant data is smaller than 256 bytes");
		Initialize(extent, swapFormat);
	}

	DeferredRenderSystem::~DeferredRenderSystem()
	{
		vkDestroyBuffer(m_Device.device(), m_LightsBuffer, nullptr); 
		vkFreeMemory(m_Device.device(), m_LightsBufferMemory, nullptr);
		vkDestroyDescriptorPool(m_Device.device(), m_LightingPassDescriptorPool, nullptr); 
		vkDestroyDescriptorSetLayout(m_Device.device(), m_LightingPassDescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.device(), m_LightPipelineLayout, nullptr);
		vkDestroyDescriptorPool(m_Device.device(), m_BlitDescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_Device.device(), m_BlitDescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.device(), m_BlitPipelineLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.device(), m_GeometryPipelineLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.device(), m_DepthPrepassPipelineLayout, nullptr);
		Texture::cleanupBindless(m_Device);
	}

	void DeferredRenderSystem::Initialize(VkExtent2D extent, VkFormat swapFormat)
	{
		m_GBuffer.create(m_Device, extent.width, extent.height);
		m_LightingPassBuffer.create(m_Device, extent.width, extent.height); 

		CreateDepthPrepassPipelineLayout();
		CreateDepthPrepassPipeline();
		CreateGeometryPipelineLayout();
		CreateGeometryPipeline();
		CreateLightingPipelineLayout();
		CreateLightingPipeline();
		CreateLightingDescriptorSet();
		CreateLightsBuffer(m_CPULights.size()); 

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

		vkDestroyDescriptorPool(m_Device.device(), m_LightingPassDescriptorPool, nullptr);
		CreateLightingDescriptorSet();
	}
#pragma endregion

#pragma region LIGHTING_PIPELINE
	void DeferredRenderSystem::CreateLightingPipelineLayout()
	{
		// one binding of an array-of-5 combined-image-samplers
		VkDescriptorSetLayoutBinding descBinding{};
		descBinding.binding = 0; 
		descBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descBinding.descriptorCount = 5;
		descBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo dsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		dsInfo.bindingCount = 1;
		dsInfo.pBindings = &descBinding;
		vkCreateDescriptorSetLayout(m_Device.device(), &dsInfo, nullptr, &m_LightingPassDescriptorSetLayout);

		// 2) Lights set (set 1):
		VkDescriptorSetLayoutBinding bLight{};
		bLight.binding = 0;
		bLight.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bLight.descriptorCount = 1;
		bLight.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		VkDescriptorSetLayoutCreateInfo lightInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		lightInfo.bindingCount = 1;
		lightInfo.pBindings = &bLight;
		vkCreateDescriptorSetLayout(m_Device.device(), &lightInfo, nullptr, &m_PointLightsDescriptorSetLayout);


		VkDescriptorSetLayout setLayouts[] = {
			m_LightingPassDescriptorSetLayout,  // set 0
			m_PointLightsDescriptorSetLayout    // set 1
		};
		// push const for camera pos and screen texel
		VkPushConstantRange pc{};
		pc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pc.offset = 0;
		pc.size = sizeof(ResolutionCameraPush);

		VkPipelineLayoutCreateInfo plInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		plInfo.setLayoutCount = 2;
		plInfo.pSetLayouts = setLayouts;
		plInfo.pushConstantRangeCount = 1;
		plInfo.pPushConstantRanges = &pc;
		vkCreatePipelineLayout(m_Device.device(), &plInfo, nullptr, &m_LightPipelineLayout);
	}
	void DeferredRenderSystem::CreateLightingDescriptorSet()
	{
		VkDescriptorPoolSize poolSizes[2]{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = 5;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.poolSizeCount = 2;
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = 2;
		if (vkCreateDescriptorPool(m_Device.device(), &poolInfo, nullptr, &m_LightingPassDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create lighting descriptor pool");
		}

		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = m_LightingPassDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_LightingPassDescriptorSetLayout;
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


		// 3) Allocate & write the Lights set (set 1):
		VkDescriptorSetAllocateInfo alloc1{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		alloc1.descriptorPool = m_LightingPassDescriptorPool;
		alloc1.descriptorSetCount = 1;
		alloc1.pSetLayouts = &m_PointLightsDescriptorSetLayout;
		vkAllocateDescriptorSets(m_Device.device(), &alloc1, &m_PointLightsDescriptorSet);

		VkDescriptorBufferInfo bufInfo{};
		bufInfo.buffer = m_LightsBuffer;
		bufInfo.offset = 0;
		bufInfo.range = sizeof(PointLight) * m_MaxLights;

		VkWriteDescriptorSet writeBuf{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeBuf.dstSet = m_PointLightsDescriptorSet;
		writeBuf.dstBinding = 0;
		writeBuf.descriptorCount = 1;
		writeBuf.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		writeBuf.pBufferInfo = &bufInfo;

		vkUpdateDescriptorSets(m_Device.device(), 1, &writeBuf, 0, nullptr);
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
		// copy into ssbo
		uint32_t count = std::min((size_t)m_CPULights.size(), m_MaxLights);
		void* ptr = nullptr;
		vkMapMemory(m_Device.device(), m_LightsBufferMemory, 0,
			sizeof(PointLight) * count, 0, &ptr);
		memcpy(ptr, m_CPULights.data(), sizeof(PointLight) * count);
		vkUnmapMemory(m_Device.device(), m_LightsBufferMemory); 

		ResolutionCameraPush pushConstantData;
		pushConstantData.resolution = glm::vec2(
			static_cast<float>(extent.width),
			static_cast<float>(extent.height)
		);
		pushConstantData.cameraPos = camera.GetPosition();


		VkDescriptorSet sets[] = { m_LightDescriptorSet, m_PointLightsDescriptorSet };

		vkCmdBindDescriptorSets(cb,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_LightPipelineLayout, 0, 2, sets, 0, nullptr);

		vkCmdPushConstants(cb,
			m_LightPipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(pushConstantData),
			&pushConstantData);

		m_LightPipeline->Bind(cb);
		// draw triangle trick
		vkCmdDraw(cb, 3, 1, 0, 0); 
	}

	void DeferredRenderSystem::CreateLightsBuffer(size_t maxLights)
	{
		m_MaxLights = maxLights;
		VkDeviceSize bufferSize = sizeof(PointLight) * m_MaxLights;
		m_Device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_LightsBuffer,
			m_LightsBufferMemory
		);
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

		
		VkPipelineLayoutCreateInfo plInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		plInfo.setLayoutCount = 1;
		plInfo.pSetLayouts = &m_BlitDescriptorSetLayout;
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
		imageInfo.sampler = m_LightingPassBuffer.getSampler();
		imageInfo.imageView = m_LightingPassBuffer.getImageView();
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

		// Bind blit/tone-map pipeline
		m_BlitPipeline->Bind(commandBuffer);
		vkCmdBindDescriptorSets(
			commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_BlitPipelineLayout, 0, 1,
			&m_BlitDescriptorSet, 0, nullptr);

		// Fullscreen triangle
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}

#pragma endregion


}