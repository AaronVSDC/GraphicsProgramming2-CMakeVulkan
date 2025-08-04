#include "Application.h"
#include "DeferredRenderSystem.h"
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
    VkExtent2D currentExtent = m_Window.GetExtent();
    DeferredRenderSystem DeferredRenderSystem = { m_Device, currentExtent, m_Renderer.GetSwapChainImageFormat() };
	Camera camera{};
    camera.SetViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f)); 

    auto viewerObject = GameObject::CreateGameObject();  
    KeyboardMovementController cameraController{}; 

    auto currentTime = std::chrono::high_resolution_clock::now(); 

    float fpsTimer = 0.0f;
    int   frameCount = 0;

    //main loop
	while (!m_Window.ShouldClose())
	{
        //TODO: to make the resizing smoother find a way to continue to draw frames while resizing,this is probably blocked now.
        // Use the "window refresh callback" to redraw the contents of your window when necessary during resizing
        //glfwSetWindowRefreshCallback() ?

        glfwPollEvents();
        VkExtent2D newExtent = m_Window.GetExtent();

        if (newExtent.width != currentExtent.width || newExtent.height != currentExtent.height) {
            DeferredRenderSystem.RecreateGBuffer(newExtent, m_Renderer.GetSwapChainImageFormat());
            currentExtent = newExtent;
        }

        auto newTime = std::chrono::high_resolution_clock::now(); 
        auto elapsedSec = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count(); 
        currentTime = newTime; 

        cameraController.MoveInPlaneXZ(m_Window.GetGLFWwindow(), elapsedSec, viewerObject); 
        camera.SetViewYXZ(viewerObject.m_Transform.translation, viewerObject.m_Transform.rotation); 


        float aspectRatio = m_Renderer.GetAspectRatio(); 
        camera.SetPerspectiveProjection(glm::radians(50.f), aspectRatio, 50.f, 3000.f); // near and far plane 

		if (auto commandBuffer = m_Renderer.BeginFrame())
		{
			m_Renderer.BeginRenderingGeometry(commandBuffer,DeferredRenderSystem.GetGBuffer() ); 
			DeferredRenderSystem.RenderGeometry(commandBuffer,m_GameObjects, camera); 
            DeferredRenderSystem.UpdateGeometry(m_GameObjects, elapsedSec); 
			m_Renderer.EndRenderingGeometry(commandBuffer, DeferredRenderSystem.GetGBuffer());


            m_Renderer.BeginRenderingLighting(commandBuffer);
            DeferredRenderSystem.RenderLighting(commandBuffer, camera, currentExtent);
            m_Renderer.EndRenderingLighting(commandBuffer);  
			m_Renderer.EndFrame(); 
		}


        //fps
        ++frameCount;
        fpsTimer += elapsedSec;
		if (fpsTimer >= 1.0f) {
            float fps = frameCount / fpsTimer;
            std::cout
                << "\rFPS: "
                << std::fixed << std::setprecision(1)
                << fps
                << "   "         
                << std::flush;

            fpsTimer -= 1.0f;
            frameCount = 0;
        }
	}

	vkDeviceWaitIdle(m_Device.device());
}

void Application::LoadGameObjects()
{
	//-------------------------------------------------------------------------
	//THIS IS WHERE ALL THE MODELS ARE LOADED (OR HARDCODED BUT PLS DONT) AND PUSHED INSIDE THE MODEL POINTER 
	//-------------------------------------------------------------------------

    std::shared_ptr<Model> sponza = Model::CreateModelFromFile(m_Device, "Resources/SponzaScene/sponza.obj");
    //std::shared_ptr<Model> erato = Model::CreateModelFromFile(m_Device, "Resources/Erato/erato.obj");



    auto gameObj = GameObject::CreateGameObject(); 
    gameObj.m_Model = sponza;
    gameObj.m_Transform.translation = { 0.f,100.f,0.f }; 
    gameObj.m_Transform.scale = glm::vec3(1.f); 
    gameObj.m_Transform.rotation = { 0.f, glm::radians(-90.f),glm::radians(180.f) };

    //auto gameObj2 = GameObject::CreateGameObject();
    //gameObj2.m_Model = erato;
    //gameObj2.m_Transform.translation = { 0.f,100.f,0.f };
    //gameObj2.m_Transform.scale = glm::vec3(2.3f);
    //gameObj2.m_Transform.rotation = { 0.f, glm::radians(-90.f),glm::radians(180.f) };

    m_GameObjects.push_back(std::move(gameObj)); 
    //m_GameObjects.push_back(std::move(gameObj2)); 

}

}
