#pragma once
//vulkan/glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


//std
#include <string>

namespace cvr
{
	class Window final
	{

	public:
		Window(const size_t width = 1200, const size_t height = 800, const std::string& windowTitle = "Vulkan Renderer");
		~Window();

		Window(const Window&) = delete;
		Window(const Window&&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(const Window&&) = delete;

		//Getters/Setters
		GLFWwindow* getWindow() const { return m_Window; }
		bool getFrameBufferResized() const { return m_FrameBufferResized; }
		void setFrameBufferResized(bool newValue) { m_FrameBufferResized = newValue; }

	private:
		//glfw window
		GLFWwindow* m_Window;
		const size_t m_WINDOW_WIDTH = 1200;
		const size_t m_WINDOW_HEIGHT = 800;


		bool m_FrameBufferResized = false;
		const std::string m_WINDOW_TITLE;
		static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);





	};

}
