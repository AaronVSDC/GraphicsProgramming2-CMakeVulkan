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
		SimpleRenderSystem(Device& device, VkFormat swapChainImageFormat, VkFormat depthFormat, const std::vector<GameObject>& gameObjects);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem& other) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem& rhs) = delete;

		void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);
		void UpdateGameObjects(std::vector<GameObject>& gameObjects, float deltaTime); 

	private:
		void CreatePipelineLayout();
		void CreatePipeline(VkFormat colorFormat, VkFormat depthFormat); 

		Device& m_Device;
		std::unique_ptr<Pipeline> m_pPipeline;
		VkPipelineLayout m_PipelineLayout;

	};

}