#pragma once
#include "Pipeline.h"
#include "Device.h"
#include "GameObject.h"
#include "Camera.h"
#include "Texture.h"
#include "GBuffer.h"



//std 
#include <memory>
#include <vector>

#include "LightBuffer.h"


namespace cve
{

	class DeferredRenderSystem final
	{
	public:
		DeferredRenderSystem(Device& device, VkExtent2D extent, VkFormat swapFormat);  
		~DeferredRenderSystem();

		DeferredRenderSystem(const DeferredRenderSystem& other) = delete;
		DeferredRenderSystem& operator=(const DeferredRenderSystem& rhs) = delete;
		DeferredRenderSystem(const DeferredRenderSystem&& other) = delete;
		DeferredRenderSystem& operator=(const DeferredRenderSystem&& rhs) = delete;

		void Initialize(VkExtent2D extent, VkFormat swapFormat); 
		void RenderGeometry(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);
		void UpdateGeometry(std::vector<GameObject>& gameObjects, float deltaTime);
		void RenderLighting(VkCommandBuffer cb, const Camera& camera, VkExtent2D extent);
		void RenderBlit(VkCommandBuffer commandBuffer); 
		void RecreateGBuffer(VkExtent2D extent, VkFormat swapFormat);
		void RenderDepthPrepass(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);

		GBuffer& GetGBuffer() { return m_GBuffer;  }
		LightBuffer& GetLightBuffer() { return m_LightBuffer; }

	private:

		void CreateDepthPrepassPipeline();
		void CreateDepthPrepassPipelineLayout();

		void CreateGeometryPipelineLayout(); 
		void CreateGeometryPipeline();

		void CreateLightingPipelineLayout();
		void CreateLightingPipeline();
		void CreateLightingDescriptorSet();


		void CreateBlitPipelineLayout();
		void CreateBlitPipeline(VkFormat swapFormat);
		void CreateBlitDescriptorSet();




		Device& m_Device;
		GBuffer						m_GBuffer;
		LightBuffer					m_LightBuffer;  
		VkPipelineLayout			m_GeometryPipelineLayout, m_LightPipelineLayout, m_DepthPrepassPipelineLayout, m_BlitPipelineLayout;
		std::unique_ptr<Pipeline>	m_GeometryPipeline, m_LightPipeline, m_DepthPrepassPipeline, m_BlitPipeline;
		VkDescriptorSet				m_GeometryDescriptorSet, m_LightDescriptorSet, m_BlitDescriptorSet; 
		VkDescriptorSetLayout		m_LightDescriptorSetLayout, m_BlitDescriptorSetLayout;
		VkDescriptorPool			m_LightDescriptorPool, m_BlitDescriptorPool;

		

		 
	};
}
