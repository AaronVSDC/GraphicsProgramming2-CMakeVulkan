#pragma once
#include <vulkan/vulkan.hpp>
#include "GLFW/glfw3.h"

namespace cvr
{

	class VulkanSurface final
	{
	public: 
		VulkanSurface(VkInstance instance, GLFWwindow* window); 
		~VulkanSurface();
		VulkanSurface(VulkanSurface&) = delete;
		VulkanSurface(VulkanSurface&&) = delete;
		VulkanSurface& operator=(VulkanSurface&) = delete; 
		VulkanSurface& operator=(VulkanSurface&&) = delete;

		VkSurfaceKHR getSurface() const { return m_Surface;  }

	private:
		void createSurface();

		VkSurfaceKHR m_Surface;
		VkInstance m_Instance;
		GLFWwindow* m_pWindow; 


	};


}
