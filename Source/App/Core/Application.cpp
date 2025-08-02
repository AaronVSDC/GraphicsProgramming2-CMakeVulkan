#include "Application.h"
#include "SimpleRenderSystem.h"
#include "Camera.h"
#include "UserInput.h"
#include "DepthPrepassRenderSystem.h"
#include "GBufferRenderSystem.h"
#include "FullScreenPassRenderSystem.h"

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
    FullScreenRenderSystem fullScreenPassSystem{ m_Device,
                                         m_Renderer.GetSwapChain(),
                                         m_Renderer.GetSwapChainImageFormat() };

    std::vector<VkFormat> gBufferFormats = { VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R16G16B16A16_SFLOAT };
    GBufferRenderSystem gBufferSystem{ m_Device,
                                       gBufferFormats,
                                       m_Renderer.GetDepthFormat() };

    DepthPrepassSystem depthPrepassSystem{ m_Device,
                                            gBufferFormats,
                                            m_Renderer.GetDepthFormat() };
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

        auto newTime = std::chrono::high_resolution_clock::now(); 
        auto elapsedSec = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count(); 
        currentTime = newTime; 

        cameraController.MoveInPlaneXZ(m_Window.GetGLFWwindow(), elapsedSec, viewerObject); 
        camera.SetViewYXZ(viewerObject.m_Transform.translation, viewerObject.m_Transform.rotation); 


        float aspectRatio = m_Renderer.GetAspectRatio(); 
        camera.SetPerspectiveProjection(glm::radians(50.f), aspectRatio, 100.f, 3000.f); // near and far plane 

		if (auto commandBuffer = m_Renderer.BeginFrame())
		{
            m_Renderer.BeginGBufferRendering(commandBuffer);
            depthPrepassSystem.RenderGameObjects(commandBuffer, m_GameObjects, camera);
            gBufferSystem.RenderGameObjects(commandBuffer, m_GameObjects, camera); 
            m_Renderer.EndGBufferRendering(commandBuffer);

            m_Renderer.BeginDynamicRendering(commandBuffer);
            fullScreenPassSystem.Render(commandBuffer, m_Renderer.GetFrameIndex());
            m_Renderer.EndDynamicRendering(commandBuffer);
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

    std::shared_ptr<Model> vikingRoomModel = Model::CreateModelFromFile(m_Device, "Resources/SponzaScene/sponza.obj");



    auto gameObj = GameObject::CreateGameObject(); 
    gameObj.m_Model = vikingRoomModel;
    gameObj.m_Transform.translation = { 0.f,100.f,0.f }; 
    gameObj.m_Transform.scale = glm::vec3(1.f); 
    gameObj.m_Transform.rotation = { 0.f, glm::radians(-90.f),glm::radians(180.f) };

    m_GameObjects.push_back(std::move(gameObj)); 

}

}
