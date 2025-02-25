#pragma once

#include "api.h"
#include "rhi/resources.h"
#include "vulkan_device.h"
#include "vulkan_swap_chain.h"
#include "vulkan_api.h"
#include <vector>
#include <memory>
#include <atomic>

// Maybe temporary solution. I will decide how much threads renderer should have
#define RENDER_THREAD_COUNT 4

namespace ad_astris::vulkan
{
	class VulkanCommandPool;
	class VulkanCommandManager;
	class VulkanQueue;
	
	class VulkanCommandBuffer
	{
		friend VulkanCommandPool;
		friend VulkanCommandManager;
		friend VulkanQueue;
		
		public:
			VulkanCommandBuffer(VulkanDevice* device, VkCommandPool pool, VkPipelineStageFlags2 waitFlag);
			~VulkanCommandBuffer();

			void wait_for_cmd(VulkanCommandBuffer* cmd)
			{
				_waitSemaphores.push_back(cmd->_signalSemaphore);
				_waitFlags.push_back(cmd->_stageFlag);
			}
		
			void add_wait_semaphore(VkSemaphore semaphore) { _waitSemaphores.push_back(semaphore); }
			VkCommandBuffer get_handle() const { return _cmdBuffer; }
		
		private:
			VulkanDevice* _device;
			VkCommandBuffer _cmdBuffer{ VK_NULL_HANDLE };
			VkSemaphore _signalSemaphore;
			VkPipelineStageFlags2 _stageFlag;
			std::vector<VkSemaphore> _waitSemaphores;
			std::vector<VkPipelineStageFlags2> _waitFlags;
			// Need to solve problem with fences
			// VkFence _cmdFence;
	};

	// One VulkanCommandPool per queue per thread per frame
	class VulkanCommandPool
	{
		friend VulkanCommandManager;
		friend VulkanQueue;
		
		public:
			VulkanCommandPool(VulkanDevice* device, VulkanQueue* queue);
			~VulkanCommandPool();

			VulkanCommandBuffer* get_cmd_buffer();
			// After calling this method, _usedCmdBuffers is cleared
			// so for the next call you record new command buffers
			void clear_after_submission();
			void flush_submitted_cmd_buffers();
			
		private:
			VulkanDevice* _device{ nullptr };
			VkCommandPool _cmdPool;
			VkPipelineStageFlags _waitFlag;
			std::vector<std::unique_ptr<VulkanCommandBuffer>> _freeCmdBuffers;
			std::vector<std::unique_ptr<VulkanCommandBuffer>> _usedCmdBuffers;
			std::vector<std::unique_ptr<VulkanCommandBuffer>> _submittedCmdBuffers;
	};
	
	void clear_after_submission(
		std::vector<std::unique_ptr<VulkanCommandPool>>& freePools,
		std::vector<std::unique_ptr<VulkanCommandPool>>& lockedPools);
	
	class VulkanCommandManager
	{
		friend VulkanQueue;
		
		public:
			VulkanCommandManager(VulkanDevice* device, VulkanSwapChain* swapChain);
			void cleanup();

			void reset_cmd_buffers(uint32_t frameIndex);
			VulkanCommandBuffer* get_command_buffer(rhi::QueueType queueType = rhi::QueueType::GRAPHICS);
			void submit(rhi::QueueType queueType, bool useSignalSemaphores);

			VkFence get_free_fence();
			void wait_fences();
			void wait_all_fences();

			void set_buffer_index(uint32_t frameIndex) { _frameIndex = frameIndex; }
		
		private:
			VulkanDevice* _device;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _freeGraphicsCmdPools;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _lockedGraphicsCmdPools;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _freeTransferCmdPools;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _lockedTransferCmdPools;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _freeComputeCmdPools;
			std::vector<std::vector<std::unique_ptr<VulkanCommandPool>>> _lockedComputeCmdPools;
			uint32_t _frameIndex{ 0 };

			class SynchronizationManager
			{
				public:
					void init(VulkanDevice* device, uint32_t bufferCount);
					void cleanup(VulkanDevice* device, uint32_t bufferCount);
					void wait_fences(VulkanDevice* device, uint32_t bufferIndex);
					//void wait_all_fences();
					VkFence get_free_fence(VulkanDevice* device, uint32_t bufferIndex);

				private:
					std::vector<std::vector<VkFence>> _freeFences;
					std::vector<std::vector<VkFence>> _lockedFences;
			};

			SynchronizationManager _syncManager;
			uint32_t _bufferCount;
		
			void flush_cmd_buffers();
	};
}
