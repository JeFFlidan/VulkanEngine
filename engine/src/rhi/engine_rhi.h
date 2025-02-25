﻿#pragma once

#include "resources.h"
#include "application_core/window.h"
#include "file_system/file_system.h"
#include <vector>
#include <array>

namespace ad_astris::rhi
{
	struct RHIInitContext
	{
		acore::IWindow* window{ nullptr };
		SwapChainInfo* swapChainInfo{ nullptr };
		GpuPreference gpuPreference{ GpuPreference::DISCRETE };
		ValidationMode validationMode{ ValidationMode::DISABLED };
	};
	
	class RHI
	{
		public:
			virtual ~RHI() = default;
		
			virtual void init(RHIInitContext& initContext) = 0;
			virtual void cleanup() = 0;
		
			virtual void create_swap_chain(SwapChain* swapChain, SwapChainInfo* info, acore::IWindow* window) = 0;
			virtual void destroy_swap_chain(SwapChain* swapChain) = 0;
			virtual void get_swap_chain_texture_views(std::vector<TextureView>& textureViews) = 0;
			virtual void reset_cmd_buffers(uint32_t currentFrameIndex) = 0;

			// Can create an empty buffer or buffer with data
			virtual void create_buffer(Buffer* buffer, BufferInfo* info, void* data = nullptr) = 0;
			virtual void create_buffer(Buffer* buffer, void* data = nullptr) = 0;
			virtual void destroy_buffer(Buffer* buffer) = 0;
			virtual void update_buffer_data(Buffer* buffer, uint64_t size, void* data) = 0;
			virtual void create_texture(Texture* texture, TextureInfo* info) = 0;
			virtual void create_texture(Texture* texture) = 0;
			virtual void create_texture_view(TextureView* textureView, TextureViewInfo* info, Texture* texture) = 0;
			virtual void create_texture_view(TextureView* textureView, Texture* texture) = 0;
			virtual void create_buffer_view(BufferView* bufferView, BufferViewInfo* info, Buffer* buffer) = 0;
			virtual void create_buffer_view(BufferView* bufferView, Buffer* buffer) = 0;
			virtual void create_sampler(Sampler* sampler, SamplerInfo* info) = 0;
			virtual void create_shader(Shader* shader, ShaderInfo* shaderInfo) = 0;
			virtual void create_render_pass(RenderPass* renderPass, RenderPassInfo* passInfo) = 0;
			virtual void create_graphics_pipeline(Pipeline* pipeline, GraphicsPipelineInfo* info) = 0;
			virtual void create_compute_pipeline(Pipeline* pipeline, ComputePipelineInfo* info) = 0;

			virtual uint32_t get_descriptor_index(Buffer* buffer) = 0;
			virtual uint32_t get_descriptor_index(TextureView* textureView) = 0;
			virtual uint32_t get_descriptor_index(BufferView* bufferView) = 0;
			virtual uint32_t get_descriptor_index(Sampler* sampler) = 0;
			virtual void bind_uniform_buffer(Buffer* buffer, uint32_t slot, uint32_t size = 0, uint32_t offset = 0) = 0;

			virtual void begin_command_buffer(CommandBuffer* cmd, QueueType queueType = QueueType::GRAPHICS) = 0;
			virtual void wait_command_buffer(CommandBuffer* cmd, CommandBuffer* waitForCmd) = 0;
			virtual void submit(QueueType queueType = QueueType::GRAPHICS, bool waitAfterSubmitting = false) = 0;
			virtual void present() = 0;
			virtual void wait_fences() = 0;

			// If size = 0 (default value), method will copy whole srcBuffer to dstBuffer
			virtual void copy_buffer(
				CommandBuffer* cmd,
				Buffer* srcBuffer,
				Buffer* dstBuffer,
				uint32_t size = 0,
				uint32_t srcOffset = 0,
				uint32_t dstOffset = 0) = 0;
			// srcTexture must have ResourceLayout TRANSFER_SRC, dstTexture must have ResourceLayout TRANSFER_DST
			virtual void copy_texture(CommandBuffer* cmd, Texture* srcTexture, Texture* dstTexture) = 0;
			// srcTexture should have ResourceLayout TRANSFER_SRC, dstTexture should have ResourceLayout TRANSFER_DST
			virtual void blit_texture(
				CommandBuffer* cmd,
				Texture* srcTexture,
				Texture* dstTexture,
				const std::array<int32_t, 3>& srcOffset,
				const std::array<int32_t, 3>& dstOffset,
				uint32_t srcMipLevel = 0,
				uint32_t dstMipLevel = 0,
				uint32_t srcBaseLayer = 0,
				uint32_t dstBaseLayer = 0) = 0;
			virtual void copy_buffer_to_texture(
				CommandBuffer* cmd,
				Buffer* srcBuffer,
				Texture* dstTexture) = 0;
			virtual void copy_texture_to_buffer(CommandBuffer* cmd, Texture* srcTexture, Buffer* dstBuffer) = 0;
			virtual void set_viewports(CommandBuffer* cmd, std::vector<Viewport>& viewports) = 0;
			virtual void set_scissors(CommandBuffer* cmd, std::vector<Scissor>& scissors) = 0 ;
			virtual void push_constants(CommandBuffer* cmd, Pipeline* pipeline, void* data) = 0;
			// Can bind only one vertex buffer.
			virtual void bind_vertex_buffer(CommandBuffer* cmd, Buffer* buffer) = 0;
			virtual void bind_index_buffer(CommandBuffer* cmd, Buffer* buffer) = 0;
			virtual void bind_pipeline(CommandBuffer* cmd, Pipeline* pipeline) = 0;
			virtual void begin_render_pass(CommandBuffer* cmd, RenderPass* renderPass, ClearValues& clearValues) = 0;
			virtual void end_render_pass(CommandBuffer* cmd) = 0;
			// No render passes
			virtual void begin_rendering(CommandBuffer* cmd, RenderingBeginInfo* beginInfo) = 0;
			// No render passes
			virtual void begin_rendering(CommandBuffer* cmd, SwapChain* swapChain, ClearValues* clearValues) = 0;
			// No render passes
			virtual void end_rendering(CommandBuffer* cmd) = 0;
			virtual void end_rendering(CommandBuffer* cmd, SwapChain* swapChain) = 0;
			// One buffer - one object
			virtual void draw(CommandBuffer* cmd, uint64_t vertexCount) = 0;
			virtual void draw_indexed(
				CommandBuffer* cmd,
				uint32_t indexCount,
				uint32_t instanceCount,
				uint32_t firstIndex,
				int32_t vertexOffset,
				uint32_t firstInstance) = 0;
			virtual void draw_indirect(CommandBuffer* cmd, Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) = 0;
			virtual void draw_indexed_indirect(CommandBuffer* cmd, Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) = 0;
			virtual void dispatch(CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
			virtual void fill_buffer(CommandBuffer* cmd, Buffer* buffer, uint32_t dstOffset, uint32_t size, uint32_t data) = 0;
			virtual void add_pipeline_barriers(CommandBuffer* cmd, const std::vector<PipelineBarrier>& barriers) = 0;

			virtual void wait_for_gpu() = 0;

			virtual void create_query_pool(QueryPool* queryPool, QueryPoolInfo* queryPoolInfo) = 0;
			virtual void create_query_pool(QueryPool* queryPool) = 0;
			virtual void begin_query(const CommandBuffer* cmd, const QueryPool* queryPool, uint32_t queryIndex) = 0;
			virtual void end_query(const CommandBuffer* cmd, const QueryPool* queryPool, uint32_t queryIndex) = 0;
			virtual void get_query_pool_result(
				const QueryPool* queryPool,
				std::vector<uint64_t>& outputData,
				uint32_t queryIndex,
				uint32_t queryCount,
				uint32_t stride) = 0;
			virtual void copy_query_pool_results(
				const CommandBuffer* cmd,
				const QueryPool* queryPool,
				uint32_t firstQuery,
				uint32_t queryCount,
				uint32_t stride,
				const Buffer* dstBuffer,
				uint32_t dstOffset = 0) = 0;
			virtual void reset_query(const CommandBuffer* cmd, const QueryPool* queryPool, uint32_t queryIndex, uint32_t queryCount) = 0;
		
			uint32_t get_buffer_count() const { return _gpuProperties.bufferCount; }
			GpuCapability get_gpu_capabilities() const { return _gpuProperties.capabilities; }
			uint64_t get_shader_identifier_size() const { return _gpuProperties.shaderIdentifierSize; }
			uint64_t get_acceleration_structure_instance_size() const { return _gpuProperties.accelerationStructureInstanceSize; }
			uint64_t get_timestamp_frequency() const { return _gpuProperties.timestampFrequency; }
			uint32_t get_vendor_id() const { return _gpuProperties.vendorID; }
			uint32_t get_device_id() const { return _gpuProperties.deviceID; }
			const std::string& get_gpu_name() const { return _gpuProperties.gpuName; }
			const std::string& get_driver_description() const { return _gpuProperties.driverDescription; }
			
			bool is_validation_enabled() const { return _gpuProperties.validationMode != ValidationMode::DISABLED; }
			bool has_capability(GpuCapability capability) const { return has_flag(_gpuProperties.capabilities, capability); }
			
			virtual GPUMemoryUsage get_memory_usage() = 0;

		protected:
			GpuProperties _gpuProperties;
	};
}
