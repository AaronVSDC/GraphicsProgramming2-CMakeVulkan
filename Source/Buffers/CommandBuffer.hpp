#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include "../Utils/Globals.hpp"
#include "../Core/Device.hpp"
#include "../Graphics/RenderPass.hpp"
#include "FrameBuffer.hpp"
#include "../Graphics/GraphicsPipeline.hpp"
#include "IndexBuffer.hpp"
#include "VertexBuffer.hpp"

namespace cvr
{
	class CommandBuffer final
	{
	public: 

		CommandBuffer(Device* device,
			RenderPass* renderPass,
			FrameBuffer* frameBuffer,
			Swapchain* swapchain,
			GraphicsPipeline* graphicsPipeline,
			VertexBuffer* vertexBuffer,
			IndexBuffer* indexBuffer,
			DescriptorManager* descriptors)
			:m_Device{device}, 
			m_RenderPass{renderPass}, 
			m_FrameBuffer{frameBuffer}, 
			m_Swapchain{swapchain}, 
			m_GraphicsPipeline{graphicsPipeline}, 
			m_VertexBuffer{vertexBuffer}, 
			m_IndexBuffer{indexBuffer}, 
			m_Descriptors{descriptors}
		{
			createCommandBuffers();  
		}
		~CommandBuffer() = default; 
		CommandBuffer(const CommandBuffer&) = delete; 
		CommandBuffer(const CommandBuffer&&) = delete; 
		CommandBuffer& operator=(const CommandBuffer&) = delete; 
		CommandBuffer& operator=(const CommandBuffer&&) = delete; 

		std::vector<VkCommandBuffer>& getCommandBuffers() { return m_CommandBuffers; } 


		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");

			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_RenderPass->getRenderPass();
			renderPassInfo.framebuffer = m_FrameBuffer->getSwapchainFrameBuffers()[imageIndex];

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_Swapchain->getSwapchainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearValues[1].depthStencil = { 1.0f, 0 };	renderPassInfo.clearValueCount = 1;
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(m_Swapchain->getSwapchainExtent().width);
			viewport.height = static_cast<float>(m_Swapchain->getSwapchainExtent().height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = m_Swapchain->getSwapchainExtent();
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->getGraphicsPipeline());

			VkBuffer vertexBuffers[] = { m_VertexBuffer->getVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->getPipelineLayout(), 0, 1, &m_Descriptors->getDescriptorSets()[currentFrame], 0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_IndexBuffer->getIndices().size()), 1, 0, 0, 0);

			vkCmdEndRenderPass(commandBuffer);
			if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}

	private: 
		std::vector<VkCommandBuffer> m_CommandBuffers;
		Device* m_Device;
		RenderPass* m_RenderPass; 
		FrameBuffer* m_FrameBuffer; 
		Swapchain* m_Swapchain;
		GraphicsPipeline* m_GraphicsPipeline; 
		VertexBuffer* m_VertexBuffer; 
		IndexBuffer* m_IndexBuffer; 
		DescriptorManager* m_Descriptors; 

		void createCommandBuffers()
		{
			m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_Device->getCommandPool();
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

			if (vkAllocateCommandBuffers(m_Device->getDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffers!");
			}

		}



	};
	
}