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
		void RecreateGBuffer(VkExtent2D extent, VkFormat swapFormat);
		void RenderDepthPrepass(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);

		GBuffer& GetGBuffer() { return m_GBuffer;  }

	private:
		void CreateGeometryPipelineLayout(); 
		void CreateGeometryPipeline();
		void CreateLightingPipelineLayout();
		void CreateLightingPipeline(VkFormat swapFormat);
		void CreateLightingDescriptorSet();

		void CreateDepthPrepassPipeline();
		void CreateDepthPrepassPipelineLayout();




		Device& m_Device;
		GBuffer						m_GBuffer; 
		VkPipelineLayout			m_GeometryPipelineLayout, m_LightPipelineLayout, m_DepthPrepassPipelineLayout;
		std::unique_ptr<Pipeline>	m_GeometryPipeline, m_LightPipeline, m_DepthPrepassPipeline;
		VkDescriptorSet				m_GeometryDescriptorSet, m_LightDescriptorSet; 
		VkDescriptorSetLayout		m_LightDescriptorSetLayout;
		VkDescriptorPool			m_LightDescriptorPool;

	};
}
