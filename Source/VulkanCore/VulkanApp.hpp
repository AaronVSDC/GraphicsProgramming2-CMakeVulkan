#pragma once
#include "InitVulkan.hpp"


//namespace: CustomVulkanRenderer
namespace cvr
{
	class VulkanApp final 
	{
	public: 
		VulkanApp() = default; 
		~VulkanApp() = default; 

		VulkanApp(const VulkanApp&) = delete; 
		VulkanApp& operator=(const VulkanApp&) = delete; 
		VulkanApp(const VulkanApp&&) = delete;
		VulkanApp& operator=(const VulkanApp&&) = delete;



		void run(); 


	private: 

		//Initialize Vulkan (everything is in the constructor)
		InitVulkan m_InitVulkan; 
	









	};







}



