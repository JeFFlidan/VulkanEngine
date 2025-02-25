#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "vulkan_common.h"
#include "rhi/utils.h"
#include "profiler/logger.h"

using namespace ad_astris;

vulkan::VulkanPipeline::VulkanPipeline(
	VulkanDevice* device,
	rhi::GraphicsPipelineInfo* info,
	VkPipelineCache pipelineCache,
	VulkanPipelineLayoutCache* layoutCache) : _device(device)
{
	create_graphics_pipeline(info, pipelineCache, layoutCache);
	_type = rhi::PipelineType::GRAPHICS;
}

vulkan::VulkanPipeline::VulkanPipeline(
	VulkanDevice* device,
	rhi::ComputePipelineInfo* info,
	VkPipelineCache pipelineCache,
	VulkanPipelineLayoutCache* layoutCache) : _device(device)
{
	create_compute_pipeline(info, pipelineCache, layoutCache);
	_type = rhi::PipelineType::COMPUTE;
}

void vulkan::VulkanPipeline::destroy(VulkanDevice* device)
{
	vkDestroyPipeline(device->get_device(), _pipeline, nullptr);
}

void vulkan::VulkanPipeline::bind(VkCommandBuffer cmd, uint32_t frameIndex)
{
	VkPipelineBindPoint bindPoint;
	switch (_type)
	{
		case rhi::PipelineType::GRAPHICS:
			bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			break;
		case rhi::PipelineType::COMPUTE:
			bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
			break;
		case rhi::PipelineType::RAY_TRACING:
			bindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
			break;
		case rhi::PipelineType::UNDEFINED:
			LOG_FATAL("VulkanPipeline::bind(): Can't bind undefined pipeline")
	}
	
	vkCmdBindPipeline(cmd, bindPoint, _pipeline);
	_layout->bind_descriptor_sets(cmd, frameIndex, bindPoint);
}

//private
void vulkan::VulkanPipeline::create_graphics_pipeline(
	rhi::GraphicsPipelineInfo* info,
	VkPipelineCache pipelineCache,
	VulkanPipelineLayoutCache* layoutCache)
{
	VkPipelineInputAssemblyStateCreateInfo assemblyState{};
	assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyState.topology = get_primitive_topology(info->assemblyState.topologyType);
	assemblyState.primitiveRestartEnable = VK_FALSE;

	// Only dynamic viewports, no static
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pScissors = nullptr;
	viewportState.scissorCount = 1;
	viewportState.pViewports = nullptr;
	viewportState.viewportCount = 1;

	if (info->rasterizationState.cullMode == rhi::CullMode::UNDEFINED)
		LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined cull mode. Failed to create VkPipeline")
	
	if (info->rasterizationState.polygonMode == rhi::PolygonMode::UNDEFINED)
		LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined polygon mode. Failed to create VkPipeline")
		
	if (info->rasterizationState.frontFace == rhi::FrontFace::UNDEFINED)
		LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined front face. Failed to create VkPipeline")

	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = get_polygon_mode(info->rasterizationState.polygonMode);
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.cullMode = get_cull_mode(info->rasterizationState.cullMode);
	rasterizationState.frontFace = get_front_face(info->rasterizationState.frontFace);
	rasterizationState.depthBiasEnable = info->rasterizationState.isBiasEnabled ? VK_TRUE : VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0.0f;
	rasterizationState.depthBiasClamp = 0.0f;
	rasterizationState.depthBiasSlopeFactor = 0.0f;

	if (info->multisampleState.sampleCount == rhi::SampleCount::UNDEFINED)
		LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined sample count. Failed to create VkPipeline")

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = info->multisampleState.isEnabled ? VK_TRUE : VK_FALSE;
	multisampleState.rasterizationSamples = get_sample_count(info->multisampleState.sampleCount);
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = info->multisampleState.isEnabled ? VK_TRUE : VK_FALSE;
	multisampleState.alphaToOneEnable = info->multisampleState.isEnabled ? VK_TRUE : VK_FALSE;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	for (auto& desc : info->bindingDescriptrions)
	{
		VkVertexInputBindingDescription description;
		description.binding = desc.binding;
		description.stride = desc.stride;
		description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescriptions.push_back(description);
	}

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	for (auto& desc : info->attributeDescriptions)
	{
		if (desc.format == rhi::Format::UNDEFINED)
			LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined format. Failed to create VkPipeline")
		
		VkVertexInputAttributeDescription description;
		description.binding = desc.binding;
		description.format = get_format(desc.format);
		description.location = desc.location;
		description.offset = desc.offset;
		attributeDescriptions.push_back(description);
	}

	VkPipelineVertexInputStateCreateInfo inputState{};
	inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	inputState.pVertexBindingDescriptions = !bindingDescriptions.empty() ? bindingDescriptions.data() : nullptr;
	inputState.vertexBindingDescriptionCount = bindingDescriptions.size();
	inputState.pVertexAttributeDescriptions = !attributeDescriptions.empty() ? attributeDescriptions.data() : nullptr;
	inputState.vertexAttributeDescriptionCount = attributeDescriptions.size();

	std::vector<VkDynamicState> dynamicStates;
	dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	if (info->rasterizationState.isBiasEnabled)
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	for (auto& attach : info->colorBlendState.colorBlendAttachments)
	{
		VkPipelineColorBlendAttachmentState attachmentState{};
		attachmentState.colorWriteMask = (VkColorComponentFlags)attach.colorWriteMask;
		if (attach.isBlendEnabled)
		{
			if (attach.srcColorBlendFactor == rhi::BlendFactor::UNDEFINED)
				LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined src color blend factor. Failed to create VkPipeline")
			
			if (attach.dstColorBlendFactor == rhi::BlendFactor::UNDEFINED)
				LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined dst color blend factor. Failed to create VkPipeline")
			
			if (attach.srcAlphaBlendFactor == rhi::BlendFactor::UNDEFINED)
				LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined src alpha blend factor. Failed to create VkPipeline")
			
			if (attach.dstAlphaBlendFactor == rhi::BlendFactor::UNDEFINED)
				LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined dst alpha blend factor. Failed to create VkPipeline")
			
			if (attach.colorBlendOp == rhi::BlendOp::UNDEFINED)
				LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined color blend op. Failed to create VkPipeline")
			
			if (attach.alphaBlendOp == rhi::BlendOp::UNDEFINED)
				LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined alpha blend op. Failed to create VkPipeline")
			
			attachmentState.blendEnable = VK_TRUE;
			attachmentState.srcColorBlendFactor = get_blend_factor(attach.srcColorBlendFactor);
			attachmentState.dstColorBlendFactor = get_blend_factor(attach.dstColorBlendFactor);
			attachmentState.srcAlphaBlendFactor = get_blend_factor(attach.srcAlphaBlendFactor);
			attachmentState.dstAlphaBlendFactor = get_blend_factor(attach.dstAlphaBlendFactor);
			attachmentState.colorBlendOp = get_blend_op(attach.colorBlendOp);
			attachmentState.alphaBlendOp = get_blend_op(attach.alphaBlendOp);
		}
		attachmentState.blendEnable = VK_FALSE;
		colorBlendAttachments.push_back(attachmentState);
	}

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = colorBlendAttachments.size();
	colorBlendState.pAttachments = colorBlendAttachments.data();
	colorBlendState.logicOpEnable = info->colorBlendState.isLogicOpEnabled ? VK_TRUE : VK_FALSE;
	colorBlendState.logicOp = get_logic_op(info->colorBlendState.logicOp);
	colorBlendState.blendConstants[0] = 1.0f;
	colorBlendState.blendConstants[1] = 1.0f;
	colorBlendState.blendConstants[2] = 1.0f;
	colorBlendState.blendConstants[3] = 1.0f;
	
	std::vector<VkPipelineShaderStageCreateInfo> pipelineStages; 
	for (auto& shader : info->shaderStages)
	{
		if (shader.type == rhi::ShaderType::UNDEFINED)
			LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Shader type is undefined")
		
		VulkanShader* vulkanShader = static_cast<VulkanShader*>(shader.handle);
		VkPipelineShaderStageCreateInfo shaderStage{};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.module = vulkanShader->get_shader_module();
		shaderStage.stage = (VkShaderStageFlagBits)get_shader_stage(shader.type);
		shaderStage.pName = "main";
		shaderStage.pSpecializationInfo = nullptr;
		pipelineStages.push_back(shaderStage);
	}

	if (info->depthStencilState.compareOp == rhi::CompareOp::UNDEFINED)
		LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined compare op, depth. Failed to create VkPipeline")
	
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = info->depthStencilState.isDepthTestEnabled ? VK_TRUE : VK_FALSE;
	depthStencilState.depthWriteEnable = info->depthStencilState.isDepthWriteEnabled ? VK_TRUE : VK_FALSE;
	depthStencilState.depthCompareOp = get_compare_op(info->depthStencilState.compareOp);
	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;
	depthStencilState.stencilTestEnable = VK_FALSE;
	std::vector<rhi::StencilOpState> stencilOps = { info->depthStencilState.backStencil, info->depthStencilState.frontStencil };
	if (info->depthStencilState.isStencilTestEnabled)
	{
		depthStencilState.stencilTestEnable = VK_TRUE;

		std::vector<VkStencilOpState> vkStencilStates;
		for (auto& stencilState : stencilOps)
		{
			if (stencilState.compareOp == rhi::CompareOp::UNDEFINED)
				LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined compare op, stencil. Failed to create VkPipeline")
			
			if (stencilState.failOp == rhi::StencilOp::UNDEFINED)
				LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined fail op stencil. Failed to create VkPipeline")
			
			if (stencilState.passOp == rhi::StencilOp::UNDEFINED)
				LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined pass op stencil. Failed to create VkPipeline")
			
			if (stencilState.depthFailOp == rhi::StencilOp::UNDEFINED)
				LOG_FATAL("VulkanPipeline::create_graphics_pipeline(): Undefined depth fail op stencil. Failed to create VkPipeline")
			
			VkStencilOpState stencilOp{};
			stencilOp.failOp = get_stencil_op(stencilState.failOp);
			stencilOp.passOp = get_stencil_op(stencilState.passOp);
			stencilOp.depthFailOp = get_stencil_op(stencilState.depthFailOp);
			stencilOp.compareOp = get_compare_op(stencilState.compareOp);
			stencilOp.compareMask = stencilState.compareMask;
			stencilOp.writeMask = stencilState.writeMask;
			stencilOp.reference = stencilState.reference;
			vkStencilStates.push_back(stencilOp);
		}
	}

	_layout = layoutCache->get_layout(info->shaderStages);

	
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.renderPass = VK_NULL_HANDLE;
	pipelineInfo.layout = _layout->get_handle();
	if (info->renderPass.handle)
	{
		pipelineInfo.renderPass = get_vk_obj(&info->renderPass)->get_handle();
	}
	pipelineInfo.subpass = 0;
	pipelineInfo.stageCount = pipelineStages.size();
	pipelineInfo.pStages = pipelineStages.data();
	pipelineInfo.pVertexInputState = &inputState;
	pipelineInfo.pInputAssemblyState = &assemblyState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.pColorBlendState = &colorBlendState;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	
	VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
	pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	pipelineRenderingCreateInfo.viewMask = 0;
	//pipelineRenderingCreateInfo.colorAttachmentCount = info->colorAttachmentFormats.size();
	
	std::vector<VkFormat> colorAttachFormats;
	for (auto& rhiFormat : info->colorAttachmentFormats)
		colorAttachFormats.push_back(get_format(rhiFormat));
	
	pipelineRenderingCreateInfo.pColorAttachmentFormats = colorAttachFormats.size() ? colorAttachFormats.data() : nullptr;
	pipelineRenderingCreateInfo.colorAttachmentCount = colorAttachFormats.size();
	if (info->depthFormat != rhi::Format::UNDEFINED)
	{
		pipelineRenderingCreateInfo.depthAttachmentFormat = get_format(info->depthFormat);
		if (rhi::Utils::support_stencil(info->depthFormat))
			pipelineRenderingCreateInfo.stencilAttachmentFormat = get_format(info->depthFormat);
	}
	
	pipelineInfo.pNext =  &pipelineRenderingCreateInfo;
	
	VK_CHECK(vkCreateGraphicsPipelines(_device->get_device(), pipelineCache, 1, &pipelineInfo, nullptr, &_pipeline));
}

void vulkan::VulkanPipeline::create_compute_pipeline(
	rhi::ComputePipelineInfo* info,
	VkPipelineCache pipelineCache,
	VulkanPipelineLayoutCache* layoutCache)
{
	VkShaderStageFlagBits stageBit = (VkShaderStageFlagBits)get_shader_stage(info->shaderStage.type);
	VulkanShader* vkShader = static_cast<VulkanShader*>(info->shaderStage.handle); 
	VulkanShaderStages vkStage;
	vkStage.add_stage(vkShader, stageBit);

	VkPipelineShaderStageCreateInfo shaderStage{};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.module = vkShader->get_shader_module();
	shaderStage.stage = stageBit;
	shaderStage.pName = "main";
	
	_layout = layoutCache->get_layout(info->shaderStage);
	VkComputePipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	createInfo.layout = _layout->get_handle();
	createInfo.stage = shaderStage;
	
	VK_CHECK(vkCreateComputePipelines(_device->get_device(), pipelineCache, 1, &createInfo, nullptr, &_pipeline));
}

void vulkan::VulkanPipeline::push_constants(VkCommandBuffer cmd, void* data)
{
	_layout->push_constant(cmd, data);
}
