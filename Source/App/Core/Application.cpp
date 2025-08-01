#include "Application.h"
#include "SimpleRenderSystem.h"
#include "Camera.h"
#include "UserInput.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm\gtc\constants.hpp>

//std
#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>

namespace cve {


Application::Application()
{
	LoadGameObjects(); 
}
Application::~Application()
{
}
void Application::run()
{

	SimpleRenderSystem simpleRenderSystem = {m_Device, m_Renderer.GetSwapChainRenderPass(), m_GameObjects };
    Camera camera{};  
    camera.SetViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f)); 

    auto viewerObject = GameObject::CreateGameObject();  
    KeyboardMovementController cameraController{}; 

    auto currentTime = std::chrono::high_resolution_clock::now(); 

    //main loop
	while (!m_Window.ShouldClose())
	{
        //TODO: to make the resizing smoother find a way to continue to draw frames while resizing,this is probably blocked now.
        // Use the "window refresh callback" to redraw the contents of your window when necessary during resizing
        //glfwSetWindowRefreshCallback() ?

        glfwPollEvents();	

        auto newTime = std::chrono::high_resolution_clock::now(); 
        auto elapsedSec = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count(); 
        currentTime = newTime; 

        cameraController.MoveInPlaneXZ(m_Window.GetGLFWwindow(), elapsedSec, viewerObject); 
        camera.SetViewYXZ(viewerObject.m_Transform.translation, viewerObject.m_Transform.rotation); 


        float aspectRatio = m_Renderer.GetAspectRatio(); 
        camera.SetPerspectiveProjection(glm::radians(50.f), aspectRatio, 0.1f, 1000.f); // near and far plane 

		if (auto commandBuffer = m_Renderer.BeginFrame())
		{
			m_Renderer.BeginSwapChainRenderPass(commandBuffer); 
			simpleRenderSystem.RenderGameObjects(commandBuffer,m_GameObjects, camera); 
            simpleRenderSystem.UpdateGameObjects(m_GameObjects, elapsedSec); 
			m_Renderer.EndSwapChainRenderPass(commandBuffer); 
			m_Renderer.EndFrame(); 
		}
	}

	vkDeviceWaitIdle(m_Device.device());
}

void Application::LoadGameObjects()
{
	//-------------------------------------------------------------------------
	//THIS IS WHERE ALL THE MODELS ARE LOADED (OR HARDCODED BUT PLS DONT) AND PUSHED INSIDE THE MODEL POINTER 
	//-------------------------------------------------------------------------

    std::shared_ptr<Model> vikingRoomModel = Model::CreateModelFromFile(m_Device, "Resources/SponzaScene/sponza.obj");



    auto gameObj = GameObject::CreateGameObject(); 
    gameObj.m_Model = vikingRoomModel;
    gameObj.m_Transform.translation = { 0.f,100.f,0.f }; 
    gameObj.m_Transform.scale = glm::vec3(1.f); 
    gameObj.m_Transform.rotation = { 0.f, 0.f,glm::radians(180.f) };

    m_GameObjects.push_back(std::move(gameObj)); 

}

}