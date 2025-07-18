#include "VulkanEngine.hpp"


namespace cve {

void VulkanEngine::run()
{

	//main render loop
	while (!glfwWindowShouldClose(m_InitVulkan.GetWindow()))
	{
		glfwPollEvents(); 
		m_InitVulkan.drawFrame(); 

	}

	vkDeviceWaitIdle(m_InitVulkan.getDevice());



}



}