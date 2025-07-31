#pragma once
#include "Window.h"
#include "Device.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Texture.h"
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
	void LoadGameObjects(); 

	static constexpr int m_WIDTH = 1080; 
	static constexpr int m_HEIGHT = 720; 

	Window m_Window{"Graphics_Programming_2_VulkanRenderer", m_WIDTH, m_HEIGHT};
	Device m_Device{m_Window}; 
	Renderer m_Renderer{ m_Window, m_Device }; 
	std::vector<GameObject> m_GameObjects;
	std::vector<std::unique_ptr<Texture>> m_Textures;

};

}