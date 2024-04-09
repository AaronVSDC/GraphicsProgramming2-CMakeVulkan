#pragma once
#include "Window.h"
#include "Pipeline.h"
#include "Device.h"
#include "SwapChain.h"

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
	void CreatePipelineLayout(); 
	void CreatePipeline(); 
	void CreateCommandBuffers(); 
	void DrawFrame(); 

	static constexpr unsigned int m_WIDTH = 1080; 
	static constexpr unsigned int m_HEIGHT = 720; 

	Window m_Window{"Graphics_Programming_2_PhysicsSandbox", m_WIDTH, m_HEIGHT};
	Device m_Device{m_Window}; 
	SwapChain m_SwapChain{ m_Device, m_Window.GetExtent() }; 
	std::unique_ptr<Pipeline> m_pPipeline; 
	VkPipelineLayout m_PipelineLayout; 
	std::vector<VkCommandBuffer> m_CommandBuffers; 

};

}