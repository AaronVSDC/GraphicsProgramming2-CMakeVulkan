#include "Window.hpp"

#include <stdexcept>



namespace cvr
{

	Window::Window(const size_t width, const size_t height, const std::string& windowTitle)
		:m_WINDOW_WIDTH(width), m_WINDOW_HEIGHT(height), m_WINDOW_TITLE(windowTitle)
	{
		if (!glfwInit())
			throw std::runtime_error("Failed to initialize GLFW"); 

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // tell it not to create OpenGl context, it was made for openGL

		m_Window = glfwCreateWindow(static_cast<int>(m_WINDOW_WIDTH), static_cast<int>(m_WINDOW_HEIGHT), m_WINDOW_TITLE.c_str(), nullptr, nullptr); //second last is to make it full screen
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, frameBufferResizeCallback);

	}
	Window::~Window()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();

	}
	void Window::frameBufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		app->m_FrameBufferResized = true;
	}



}
