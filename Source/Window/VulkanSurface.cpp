#include "VulkanSurface.hpp"

//std
#include <stdexcept>

namespace cvr
{
	VulkanSurface::VulkanSurface(VkInstance instance, GLFWwindow* pWindow)
		:m_Instance(instance), m_pWindow(pWindow)
	{
		createSurface(); 
	}

	VulkanSurface::~VulkanSurface()
	{
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	}

	void VulkanSurface::createSurface()
	{
		if (glfwCreateWindowSurface(m_Instance, m_pWindow, nullptr, &m_Surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan surface");
		}
	}
}
