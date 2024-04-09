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
		Window(std::string windowName, unsigned int width, unsigned int height);
		Window(Window& other) = delete;
		Window(Window&& other) = delete;
		Window& operator=(Window& rhs) = delete;
		Window& operator=(Window&& rhs) = delete;
		~Window(); 

		bool ShouldClose() { return glfwWindowShouldClose(m_Window); }
		VkExtent2D GetExtent() { return { static_cast<uint32_t>(m_WIDTH),static_cast<uint32_t>(m_HEIGHT) }; }


		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		void InitWindow(); 

		const unsigned int m_WIDTH; 
		const unsigned int m_HEIGHT; 

		std::string m_WindowName; 
		GLFWwindow* m_Window; 

	};
}
