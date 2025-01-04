#pragma once
#include "InitVulkan.hpp"


//namespace: CustomVulkanEngine
namespace cve
{
	class VulkanEngine final 
	{
	public: 
		VulkanEngine(); 
		~VulkanEngine(); 

		VulkanEngine(const VulkanEngine&) = delete; 
		VulkanEngine& operator=(const VulkanEngine&) = delete; 
		VulkanEngine(const VulkanEngine&&) = delete;
		VulkanEngine& operator=(const VulkanEngine&&) = delete;



		void run(); 


	private: 

		//Initialize Vulkan (everything is in the constructor)
		InitVulkan m_InitVulkan; 

		void drawFrame(); 









	};







}



