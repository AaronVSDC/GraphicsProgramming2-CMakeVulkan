#include "Application.h"
#include "Camera.h"
#include "UserInput.h"
#include "HDRImage.h" 

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
    DeferredRenderSystem deferredRenderSystem = { m_Device, currentExtent, m_Renderer.GetSwapChainImageFormat(),m_HDRImage,  m_Lights };
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
            deferredRenderSystem.RecreateGBuffer(newExtent, m_Renderer.GetSwapChainImageFormat());
            currentExtent = newExtent;
        }

        auto newTime = std::chrono::high_resolution_clock::now(); 
        auto elapsedSec = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count(); 
        currentTime = newTime; 

        cameraController.MoveInPlaneXZ(m_Window.GetGLFWwindow(), elapsedSec, viewerObject); 
        camera.SetViewYXZ(viewerObject.m_Transform.translation, viewerObject.m_Transform.rotation); 


        float aspectRatio = m_Renderer.GetAspectRatio(); 
        camera.SetPerspectiveProjection(glm::radians(50.f), aspectRatio, 0.1f, 50.f); // near and far plane 

		if (auto commandBuffer = m_Renderer.BeginFrame())
		{
            //depth prepass

            m_Renderer.BeginRenderingDepthPrepass(commandBuffer, deferredRenderSystem.GetGBuffer());
            deferredRenderSystem.RenderDepthPrepass(commandBuffer, m_GameObjects, camera);
            m_Renderer.EndRenderingDepthPrepass(commandBuffer);

			m_Renderer.BeginRenderingGeometry(commandBuffer,deferredRenderSystem.GetGBuffer() ); 
			deferredRenderSystem.RenderGeometry(commandBuffer,m_GameObjects, camera); 
            deferredRenderSystem.UpdateGeometry(m_GameObjects, elapsedSec); 
			m_Renderer.EndRenderingGeometry(commandBuffer, deferredRenderSystem.GetGBuffer());


            m_Renderer.BeginRenderingLighting(commandBuffer, deferredRenderSystem.GetLightBuffer());
            deferredRenderSystem.RenderLighting(commandBuffer, camera, m_Renderer.GetSwapChainExtent());
            m_Renderer.EndRenderingLighting(commandBuffer, deferredRenderSystem.GetLightBuffer());

            m_Renderer.BeginRenderingBlittingPass(commandBuffer);
            deferredRenderSystem.RenderBlit(commandBuffer);
            m_Renderer.EndRenderingBlittingPass(commandBuffer); 


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
    m_HDRImage = std::make_unique<HDRImage>(m_Device, "Resources/HDRImages/circus_arena_4k.hdr");

    std::shared_ptr<Model> newSponza = Model::CreateModelFromFile(m_Device, "Resources/ABeautifulGame/glTF/ABeautifulGame.gltf");
    auto gameObj = GameObject::CreateGameObject(); 
    gameObj.m_Model = newSponza;
    gameObj.m_Transform.translation = { 0.f,0.f,0.f }; 
    gameObj.m_Transform.scale = glm::vec3(1.f); 
    gameObj.m_Transform.rotation = { 0.f, glm::radians(-90.f),glm::radians(180.f) };
    m_GameObjects.push_back(std::move(gameObj));

    // Add a red point light at (10,10,10):


    //Light yellowLight;
    //yellowLight.type = LightType::Point;
    //yellowLight.lightIntensity = 200.f;
    //yellowLight.position = glm::vec3{ 0,0,0 };
    //yellowLight.lightColor = { 1.0f, 0.27f, .17f };
    //yellowLight.radius = { 100.f };

    //Light redLight;
    //redLight.type = LightType::Point;
    //redLight.lightIntensity = 200.f;
    //redLight.position = glm::vec3{ 0,0,6 };
    //redLight.lightColor = { .5f, 0.8f, .15f };
    //redLight.radius = { 100.f };

    //m_Lights.push_back(yellowLight); 
    //m_Lights.push_back(redLight);

    m_Lights.push_back({
        { 0.f, 0.f, 0.f },
         0.f,
         {0.577f, 0.577f, -0.577f},
         LightType::Directional,
         { 1.000, 0.891, 0.796 },
         1.f
        });
}

}
