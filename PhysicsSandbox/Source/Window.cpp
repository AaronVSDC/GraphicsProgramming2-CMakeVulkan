#include "Window.h"

//std
#include <stdexcept>


namespace cve
{

	Window::Window(std::string windowName, unsigned int width, unsigned int height)
		:m_WIDTH{width}, 
		m_HEIGHT{height}, 
		m_WindowName{windowName}
	{
		InitWindow(); 
	}



	void Window::InitWindow()
	{
		//initialize the glfw window
		glfwInit(); 

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //Tells glfw that this is not a window creation for OpenGL (or something like that) 
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //Tells glfw that the window is not resizeable for now because its a whole thing to resize the window

		//now create the window
		m_Window = glfwCreateWindow(m_WIDTH, m_HEIGHT, m_WindowName.c_str(), nullptr, nullptr); //last 2 parameters meant for full screen

	}
	

	Window::~Window()
	{
		//release everything 
		glfwDestroyWindow(m_Window); 
		glfwTerminate(); 
	}

	void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface (window class)"); 
		}

	}
}