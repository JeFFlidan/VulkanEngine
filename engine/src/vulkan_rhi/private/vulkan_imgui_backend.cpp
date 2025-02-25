﻿#include "vulkan_imgui_backend.h"
#include "vulkan_common.h"
#include "core/global_objects.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_win32.h"
#include "file_system/utils.h"
#include <stb_image/stb_image.h>

using namespace ad_astris;
using namespace vulkan;

PFN_vkVoidFunction load_function(const char* functionName, void* userData)
{
	VulkanRHI* rhi = static_cast<VulkanRHI*>(userData);
	PFN_vkVoidFunction instanceAddr = vkGetInstanceProcAddr(rhi->get_instance()->get_handle(), functionName);
	PFN_vkVoidFunction deviceAddr = vkGetDeviceProcAddr(rhi->get_device()->get_device(), functionName);
	return deviceAddr ? deviceAddr : instanceAddr;
}

void VulkanImGuiBackend::init(rhi::ImGuiBackendInitContext& initContext)
{
	_rhi = static_cast<VulkanRHI*>(initContext.rhi);
	_mainWindow = initContext.window;
	_sampler = initContext.sampler;
	
	VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = std::size(poolSizes);
	poolInfo.pPoolSizes = poolSizes;
	VK_CHECK(vkCreateDescriptorPool(_rhi->get_device()->get_device(), &poolInfo, nullptr, &_descriptorPool));

	ImGui::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = _mainWindow->get_width();
	io.DisplaySize.y = _mainWindow->get_height();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = nullptr;

#ifdef _WIN32
	ImGui_ImplWin32_LoadFunctions(load_function, _rhi);
	ImGui_ImplWin32_InitForVulkan(_mainWindow->get_hWnd());
#endif
	
	ImGui_ImplVulkan_InitInfo vulkanInitInfo{};
	vulkanInitInfo.Instance = _rhi->get_instance()->get_handle();
	vulkanInitInfo.PhysicalDevice = _rhi->get_device()->get_physical_device();
	vulkanInitInfo.Device = _rhi->get_device()->get_device();
	vulkanInitInfo.Queue = _rhi->get_device()->get_graphics_queue()->get_handle();
	vulkanInitInfo.DescriptorPool = _descriptorPool;
	vulkanInitInfo.MinImageCount = 3;
	vulkanInitInfo.ImageCount = 3;
	vulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	vulkanInitInfo.UseDynamicRendering = true;
	vulkanInitInfo.ColorAttachmentFormat = _rhi->get_swap_chain()->get_format();

	ImGui_ImplVulkan_LoadFunctions(load_function, _rhi);
	ImGui_ImplVulkan_Init(&vulkanInitInfo, VK_NULL_HANDLE);
	rhi::CommandBuffer cmdBuffer;
	
	io.Fonts->AddFontDefault();
	_defaultFont14 = io.Fonts->AddFontFromFileTTF((FILE_SYSTEM()->get_engine_root_path() + "/fonts/unispace bd.ttf").c_str(), 14.0f);
	_defaultFont17 = io.Fonts->AddFontFromFileTTF((FILE_SYSTEM()->get_engine_root_path() + "/fonts/unispace bd.ttf").c_str(), 17.0f);
	
	_rhi->begin_command_buffer(&cmdBuffer, rhi::QueueType::GRAPHICS);
	ImGui_ImplVulkan_CreateFontsTexture(get_vk_obj(&cmdBuffer)->get_handle());

	auto getIconName = [](const std::string& name) -> std::string
	{
		return "/icons/" + name + ".png";
	};

	load_texture(&cmdBuffer, getIconName(rhi::FOLDER_ICON_NAME));
	load_texture(&cmdBuffer, getIconName(rhi::TEXTURE_ICON_NAME));
	load_texture(&cmdBuffer, getIconName(rhi::MODEL_ICON_NAME));
	load_texture(&cmdBuffer, getIconName(rhi::MATERIAL_ICON_NAME));
	load_texture(&cmdBuffer, getIconName(rhi::LEVEL_ICON_NAME));

	_rhi->submit(rhi::QueueType::GRAPHICS, true);
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	for (auto& buffer : _cpuBuffers)
		_rhi->destroy_buffer(&buffer);
	_cpuBuffers.clear();

	_imguiContext = ImGui::GetCurrentContext();
	void* userData;
	ImGui::GetAllocatorFunctions(&_imguiAllocFunc, &_imguiFreeFunc, &userData);
}

void VulkanImGuiBackend::cleanup()
{
	vkDestroyDescriptorPool(_rhi->get_device()->get_device(), _descriptorPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();
}

void VulkanImGuiBackend::begin_frame() const
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = static_cast<float>(_mainWindow->get_width());
	io.DisplaySize.y = static_cast<float>(_mainWindow->get_height());
	ImGui_ImplVulkan_NewFrame();
#ifdef _WIN32
	ImGui_ImplWin32_NewFrame();
#endif
}

void VulkanImGuiBackend::draw(rhi::CommandBuffer* cmd)
{
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), get_vk_obj(cmd)->get_handle());
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void VulkanImGuiBackend::set_backbuffer(rhi::TextureView* textureView, rhi::Sampler sampler)
{
	if (_backbufferDescriptorSet == VK_NULL_HANDLE)
	{
		_backbufferDescriptorSet = ImGui_ImplVulkan_AddTexture(
		   get_vk_obj(&sampler)->get_handle(),
		   get_vk_obj(textureView)->get_handle(),
		   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	else
	{
		ImGui_ImplVulkan_UpdateTexture(
			_backbufferDescriptorSet,
			get_vk_obj(&sampler)->get_handle(),
			get_vk_obj(textureView)->get_handle(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}

void VulkanImGuiBackend::load_texture(rhi::CommandBuffer* cmd, const io::URI& texturePath)
{
	io::URI filePath = FILE_SYSTEM()->get_engine_root_path() + texturePath;
	uicore::TextureInfo textureInfo;
	unsigned char* imageData = stbi_load(filePath.c_str(), &textureInfo.width, &textureInfo.height, NULL, 4);
	if (imageData == NULL)
		LOG_FATAL("Failed to load icon {}", filePath.c_str());

	rhi::Buffer& cpuBuffer = _cpuBuffers.emplace_back();

	rhi::Texture texture;
	texture.textureInfo.width = textureInfo.width;
	texture.textureInfo.height = textureInfo.height;
	texture.textureInfo.layersCount = 1;
	texture.textureInfo.mipLevels = 1;
	texture.textureInfo.format = rhi::Format::R8G8B8A8_UNORM;
	texture.textureInfo.memoryUsage = rhi::MemoryUsage::GPU;
	texture.textureInfo.samplesCount = rhi::SampleCount::BIT_1;
	texture.textureInfo.textureDimension = rhi::TextureDimension::TEXTURE2D;
	texture.textureInfo.textureUsage = rhi::ResourceUsage::SAMPLED_TEXTURE | rhi::ResourceUsage::TRANSFER_DST;
	_rhi->create_texture(&texture);
	
	cpuBuffer.bufferInfo.size = textureInfo.width * textureInfo.height * 4;
	cpuBuffer.bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
	cpuBuffer.bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
	_rhi->create_buffer(&cpuBuffer, imageData);

	_rhi->copy_buffer_to_texture(cmd, &cpuBuffer, &texture);

	rhi::TextureView textureView;
	textureView.viewInfo.baseLayer = 0;
	textureView.viewInfo.baseMipLevel = 0;
	textureView.viewInfo.textureAspect = rhi::TextureAspect::COLOR;
	_rhi->create_texture_view(&textureView, &texture);
	
	textureInfo.textureID64 = (uint64_t)ImGui_ImplVulkan_AddTexture(
		   get_vk_obj(&_sampler)->get_handle(),
		   get_vk_obj(&textureView)->get_handle(),
		   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
	_textureByName[io::Utils::get_file_name(texturePath)] = textureInfo;
	stbi_image_free(imageData);
}

const uicore::TextureInfo& VulkanImGuiBackend::get_texture_info(const std::string& textureName) const
{
	auto it = _textureByName.find(textureName);
	if (it != _textureByName.end())
	{
		return it->second;
	}

	LOG_ERROR("VulkanImGuiBackend::get_texture_info(): No texture with name {}", textureName)
	return uicore::TextureInfo();
}
