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
		glm::uint  lightCount;
	};

	struct DepthPush
	{
		glm::mat4 mvp;
		uint32_t baseColorIndex;
	};


	struct alignas(16) PointLight
	{
		glm::vec3 position;
		float     radius;
		glm::vec3 lightColor;    
		float     lightIntensity;
	};

	class DeferredRenderSystem final
	{
	public:
		DeferredRenderSystem(Device& device, VkExtent2D extent, VkFormat swapFormat, std::vector<PointLight>& lights);
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
		LightBuffer& GetLightBuffer() { return m_LightingPassBuffer; }

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
		void CreateLightsBuffer(size_t maxLights);





		Device& m_Device;
		GBuffer						m_GBuffer;
		LightBuffer					m_LightingPassBuffer;  
		VkPipelineLayout			m_GeometryPipelineLayout, m_LightPipelineLayout, m_DepthPrepassPipelineLayout, m_BlitPipelineLayout;
		std::unique_ptr<Pipeline>	m_GeometryPipeline, m_LightPipeline, m_DepthPrepassPipeline, m_BlitPipeline;
		VkDescriptorSet				m_GeometryDescriptorSet, m_LightDescriptorSet, m_BlitDescriptorSet; 
		VkDescriptorSetLayout		m_LightingPassDescriptorSetLayout, m_BlitDescriptorSetLayout; 
		VkDescriptorPool			m_LightingPassDescriptorPool, m_BlitDescriptorPool;

		VkBuffer				m_LightsBuffer; 
		VkDeviceMemory			m_LightsBufferMemory;
		size_t					m_MaxLights = 0; 
		VkDescriptorSetLayout   m_PointLightsDescriptorSetLayout;
		VkDescriptorPool        m_PointLightsDescriptorPool;
		VkDescriptorSet         m_PointLightsDescriptorSet;

		std::vector<PointLight> m_CPULights; 

		 
	};
}
