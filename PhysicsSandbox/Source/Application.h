#pragma once
#include "Window.h"
#include "Pipeline.h"
#include "Device.h"
#include "SwapChain.h"
#include "Model.h"

//std 
#include <memory>
#include <vector>
namespace cve {

class Application
{
public: 
	Application(); 
	~Application(); 

	Application(const Application& other) = delete;
	Application& operator=(const Application& rhs) = delete; 
	Application(const Application&& other) = delete;
	Application& operator=(const Application&& rhs) = delete;

	void run();
private: 
	void LoadModels(); 
	void CreatePipelineLayout(); 
	void CreatePipeline(); 
	void CreateCommandBuffers(); 
	void FreeCommandBuffers(); 
	void DrawFrame(); 
	void RecreateSwapChain(); 
	void RecordCommandBuffer(int imageIndex); 


	static constexpr int m_WIDTH = 1080; 
	static constexpr int m_HEIGHT = 720; 

	Window m_Window{"Graphics_Programming_2_PhysicsSandbox", m_WIDTH, m_HEIGHT};
	Device m_Device{m_Window}; 
	std::unique_ptr<SwapChain> m_SwapChain; 
	std::unique_ptr<Pipeline> m_pPipeline; 
	VkPipelineLayout m_PipelineLayout; 
	std::vector<VkCommandBuffer> m_CommandBuffers; 
	std::unique_ptr<Model> m_pModel; 

};

}