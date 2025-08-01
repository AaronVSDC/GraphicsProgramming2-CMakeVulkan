#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

//namespace: "CustomVulkanEngine"
namespace cve
{
	class Window
	{
	public:
		Window(std::string windowName);
		Window(Window& other) = delete;
		Window(Window&& other) = delete;
		Window& operator=(Window& rhs) = delete;
		Window& operator=(Window&& rhs) = delete;
		~Window(); 

		bool ShouldClose() { return glfwWindowShouldClose(m_Window); }
		VkExtent2D GetExtent() { return { static_cast<uint32_t>(m_Width),static_cast<uint32_t>(m_Height) }; }

		bool WasWindowResized() const { return m_FrameBufferResized; }
		void ResetWindowResizedFlag() { m_FrameBufferResized = false;  }


		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		GLFWwindow* GetGLFWwindow() const { return m_Window;  }

	private:
		static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height); 
		void InitWindow(); 

		int m_Width = 1080; 
		int m_Height = 720; 
		bool m_FrameBufferResized = false; 

		std::string m_WindowName; 
		GLFWwindow* m_Window; 

	};
}
