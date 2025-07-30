#pragma once 
#include <vector>
#include <vulkan/vulkan.h>
#include "../Core/Device.hpp"
#include "../Buffers/UniformBuffers.hpp"
#include "../Textures/Texture.hpp"
namespace cvr
{
	class DescriptorManager
	{
	public:
		DescriptorManager(Device* device, UniformBuffers* uniformBuffers, Texture* texture); 
		~DescriptorManager(); 

		DescriptorManager(DescriptorManager&) = delete; 
		DescriptorManager(DescriptorManager&&) = delete;
		DescriptorManager& operator=(DescriptorManager&) = delete; 
		DescriptorManager& operator=(DescriptorManager&&) = delete;

		VkDescriptorSetLayout& getDescriptorSetLayout() { return m_DescriptorSetLayout;  }
		std::vector<VkDescriptorSet>& getDescriptorSets() { return  m_DescriptorSets; }
	private:

		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSets();

		//ref
		Device* m_Device;
		UniformBuffers* m_UniformBuffers;
		Texture* m_Texture; 
	};
}