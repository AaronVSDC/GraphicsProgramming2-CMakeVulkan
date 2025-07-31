#pragma once
#include "Pipeline.h"
#include "Device.h"
#include "GameObject.h"
#include "Camera.h"
#include "Texture.h"


//std 
#include <memory>
#include <vector>
namespace cve {

	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(Device& device, VkRenderPass renderPass, const std::vector<GameObject>& gameObjects);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem& other) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem& rhs) = delete;

		void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);
		void UpdateGameObjects(std::vector<GameObject>& gameObjects, float deltaTime); 

	private:
		void CreatePipelineLayout();
		void CreatePipeline(VkRenderPass renderPass);
		void SetupDescriptorSets(const std::vector<Texture*>& textures);

		Device& m_Device;
		std::unique_ptr<Pipeline> m_pPipeline;
		VkPipelineLayout m_PipelineLayout;

	};

}