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
		Window(std::string windowName, int width, int height);
		Window(Window& other) = delete;
		Window(Window&& other) = delete;
		Window& operator=(Window& rhs) = delete;
		Window& operator=(Window&& rhs) = delete;
		~Window(); 

		bool ShouldClose() { return glfwWindowShouldClose(m_Window); }
		VkExtent2D GetExtent() { return { static_cast<uint32_t>(m_Width),static_cast<uint32_t>(m_Height) }; }

		bool WasWindowResized() { return m_FrameBufferResized; }
		void ResetWindowResizedFlag() { m_FrameBufferResized = false;  }


		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height); 
		void InitWindow(); 

		int m_Width; 
		int m_Height; 
		bool m_FrameBufferResized = false; 

		std::string m_WindowName; 
		GLFWwindow* m_Window; 

	};
}
