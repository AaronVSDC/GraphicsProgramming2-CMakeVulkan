#pragma once
#include "Pipeline.h"
#include "Device.h"
#pragma once
#include "GameObject.h"
#include "Camera.h"

//std 
#include <memory>
#include <vector>
namespace cve {

	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(Device& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem& other) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem& rhs) = delete;

		void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);

	private:
		void CreatePipelineLayout();
		void CreatePipeline(VkRenderPass renderPass);

		Device& m_Device;
		std::unique_ptr<Pipeline> m_pPipeline;
		VkPipelineLayout m_PipelineLayout;

	};

}