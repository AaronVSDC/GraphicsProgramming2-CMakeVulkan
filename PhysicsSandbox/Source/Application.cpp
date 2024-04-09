#include "Application.h"

//std
#include <stdexcept>
#include <array>
namespace cve {
	Application::Application()
	{
		CreatePipelineLayout(); 
		CreatePipeline(); 
		CreateCommandBuffers(); 
	}
	Application::~Application()
	{
		vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr); 
	}
	void Application::run()
{
	while (!m_Window.ShouldClose())
	{
		glfwPollEvents();
		DrawFrame(); 
	}

	vkDeviceWaitIdle(m_Device.device()); 
}
void Application::CreatePipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{}; 
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; 
	pipelineLayoutInfo.setLayoutCount = 0; 
	pipelineLayoutInfo.pSetLayouts = nullptr; 	// is used to pass vertex data other than vertex and fragment shaders
	pipelineLayoutInfo.pushConstantRangeCount = 0; 
	pipelineLayoutInfo.pPushConstantRanges = nullptr; 
	if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout"); 
	}
}
void Application::CreatePipeline()
{
	PipelineConfigInfo pipelineConfig = Pipeline::DefaultPipelineConfigInfo(m_SwapChain.width(), m_SwapChain.height()); 
	pipelineConfig.renderPass = m_SwapChain.getRenderPass(); 
	pipelineConfig.pipelineLayout = m_PipelineLayout; 
	m_pPipeline = std::make_unique<Pipeline>(
		m_Device,
		pipelineConfig,
		"Shaders/SimpleShader.vert.spv",
		"Shaders/SimpleShader.frag.spv"); 

}
void Application::CreateCommandBuffers()
{
	m_CommandBuffers.resize(m_SwapChain.imageCount()); 

	VkCommandBufferAllocateInfo allocInfo{}; 
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; 
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
	allocInfo.commandPool = m_Device.getCommandPool(); 
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size()); 

	if (vkAllocateCommandBuffers(m_Device.device(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers (application class)"); 
	}

	for (size_t index{}; index < m_CommandBuffers.size(); ++index)
	{
		VkCommandBufferBeginInfo beginInfo{}; 
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; 

		if (vkBeginCommandBuffer(m_CommandBuffers[index], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer");
		}

		VkRenderPassBeginInfo renderPassInfo{}; 
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO; 
		renderPassInfo.renderPass = m_SwapChain.getRenderPass(); 
		renderPassInfo.framebuffer = m_SwapChain.getFrameBuffer(index); 

		renderPassInfo.renderArea.offset = { 0,0 }; 
		renderPassInfo.renderArea.extent = m_SwapChain.getSwapChainExtent(); 

		std::array<VkClearValue, 2> clearValues{}; 
		clearValues[0].color = { 0.1f,0.1f,0.1f,1.f }; 
		clearValues[1].depthStencil = { 1.f,0 }; 
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); 
		renderPassInfo.pClearValues = clearValues.data(); 


		vkCmdBeginRenderPass(m_CommandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		m_pPipeline->Bind(m_CommandBuffers[index]);
		vkCmdDraw(m_CommandBuffers[index], 3, 1, 0, 0);

		vkCmdEndRenderPass(m_CommandBuffers[index]);
		if (vkEndCommandBuffer(m_CommandBuffers[index]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer"); 
		}
	}


}
void Application::DrawFrame()
{
	uint32_t imageIndex; 
	auto result = m_SwapChain.acquireNextImage(&imageIndex); 

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image"); 
	}

	result = m_SwapChain.submitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex); 
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image"); 
	}
}
}