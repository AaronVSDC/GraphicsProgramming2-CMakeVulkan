#include "Application.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>

//std
#include <stdexcept>
#include <array>
#include <iostream>
namespace cve {


struct SimplePushConstantData
{
	glm::mat2 transform{ 1.f };
	glm::vec2 offset; 
	alignas(16)glm::vec3 color;
};

Application::Application()
{
	LoadModels(); 
	CreatePipelineLayout(); 
	RecreateSwapChain(); 
	CreateCommandBuffers(); 
}
Application::~Application()
{
	vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr); 
}
void Application::run()
{

	//this is the main loop
	while (!m_Window.ShouldClose())
	{
		glfwPollEvents(); //TODO: to make the resizing smoother find a way to continue to draw frames while resizing, this is probably blocked now
						  //TODO: use the "window refresh callback" to redraw the contents of your window when necessary during resizing
		//glfwSetWindowRefreshCallback() ?
		DrawFrame();
	}

	vkDeviceWaitIdle(m_Device.device());
}
void Application::LoadModels()
{
	//-------------------------------------------------------------------------
	//THIS IS WHERE ALL THE MODELS ARE LOADED (OR HARDCODED BUT PLS DONT) AND PUSHED INSIDE THE MODEL POINTER 
	//-------------------------------------------------------------------------
	std::vector<Model::Vertex> vertices
	{
		{{0.0f,-0.5f},{1.f,0.f,0.f}}, 
		{{0.5f,0.5f} ,{0.f,1.f,0.f}},
		{{-0.5f,0.5f} ,{0.f,0.f,1.f}}
	};

	m_pModel = std::make_unique<Model>(m_Device, vertices); //in the model class everything is set up so that the vertices are pushed in the vertex shader
}
void Application::CreatePipelineLayout()
{
	VkPushConstantRange pushConstantRange{}; 
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; 
	pushConstantRange.offset = 0; 
	pushConstantRange.size = sizeof(SimplePushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{}; 
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; 
	pipelineLayoutInfo.setLayoutCount = 0; 
	pipelineLayoutInfo.pSetLayouts = nullptr; 	// is used to pass vertex data other than vertex and fragment shaders
	pipelineLayoutInfo.pushConstantRangeCount = 1; 
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; 
	if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout"); 
	}
}
void Application::CreatePipeline()
{
	assert(m_SwapChain != nullptr && "Cannot create pipeline before swapchain"); 
	assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = m_SwapChain->getRenderPass(); 
	pipelineConfig.pipelineLayout = m_PipelineLayout; 
	m_pPipeline = std::make_unique<Pipeline>(
		m_Device,
		pipelineConfig,
		"Shaders/SimpleShader.vert.spv",
		"Shaders/SimpleShader.frag.spv"); 

}
void Application::RecreateSwapChain()
{
	auto extent = m_Window.GetExtent(); 
	while (extent.width == 0 or extent.height == 0)
	{
		extent = m_Window.GetExtent(); 
		glfwWaitEvents(); 
	}

	vkDeviceWaitIdle(m_Device.device()); 
	if (m_SwapChain == nullptr)
	{
		m_SwapChain = std::make_unique<SwapChain>(m_Device, extent);
	}
	else
	{
		m_SwapChain = std::make_unique<SwapChain>(m_Device, extent, std::move(m_SwapChain));
		if (m_SwapChain->imageCount() != m_CommandBuffers.size())
		{
			FreeCommandBuffers();
			CreateCommandBuffers(); 
		}
	}

	//if render pass compatible do nothing else (possible optimization)
	CreatePipeline(); 

}
void Application::CreateCommandBuffers()
{
	m_CommandBuffers.resize(m_SwapChain->imageCount()); 
	VkCommandBufferAllocateInfo allocInfo{}; 
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; 
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
	allocInfo.commandPool = m_Device.getCommandPool(); 
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size()); 

	if (vkAllocateCommandBuffers(m_Device.device(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers (application class)"); 
	}
}

void Application::FreeCommandBuffers()
{
	vkFreeCommandBuffers(m_Device.device(),
						 m_Device.getCommandPool(),
						 static_cast<uint32_t>(m_CommandBuffers.size()),
						 m_CommandBuffers.data());
	m_CommandBuffers.clear(); 
}

void Application::RecordCommandBuffer(int imageIndex)
{
	static int frame = 0; 
	frame = (frame + 1) % 1000; 

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(m_CommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_SwapChain->getRenderPass();
	renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer(imageIndex);

	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.01f,0.01f,0.01f,1.f }; //background color
	clearValues[1].depthStencil = { 1.f,0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();


	vkCmdBeginRenderPass(m_CommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{}; 
	viewport.x = 0.f; 
	viewport.y = 0.f; 
	viewport.width = static_cast<float>(m_SwapChain->getSwapChainExtent().width); 
	viewport.height = static_cast<float>(m_SwapChain->getSwapChainExtent().height); 
	viewport.minDepth = 0.f; 
	viewport.maxDepth = 1.f; 
	VkRect2D scissor{ {0,0}, m_SwapChain->getSwapChainExtent() }; 
	vkCmdSetViewport(m_CommandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(m_CommandBuffers[imageIndex], 0, 1, &scissor);



	m_pPipeline->Bind(m_CommandBuffers[imageIndex]);
	m_pModel->Bind(m_CommandBuffers[imageIndex]);

	for (int j{}; j < 4; j++)
	{
		SimplePushConstantData push{}; 
		push.offset = { -0.5f + frame * 0.002f,-0.4f + j * 0.25f }; 
		push.color = { 0.0f,0.0f,0.2f + 0.2f * j }; 

		vkCmdPushConstants(m_CommandBuffers[imageIndex],
			m_PipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(SimplePushConstantData),
			&push);	
		m_pModel->Draw(m_CommandBuffers[imageIndex]);
	}



	vkCmdEndRenderPass(m_CommandBuffers[imageIndex]);
	if (vkEndCommandBuffer(m_CommandBuffers[imageIndex]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer");
	}
}

void Application::DrawFrame()
{
	uint32_t imageIndex; 
	auto result = m_SwapChain->acquireNextImage(&imageIndex); 

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain(); 
		return; 
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image"); 
	}


	RecordCommandBuffer(imageIndex); 
	result = m_SwapChain->submitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex); 
	if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR or m_Window.WasWindowResized())
	{
		m_Window.ResetWindowResizedFlag(); 
		RecreateSwapChain(); 
		return; 
	}
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image"); 
	}
}
}