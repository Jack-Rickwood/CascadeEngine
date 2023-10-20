#include <stdexcept>
#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include "renderer.h"
#include "math/random/rng.h"

namespace cscd {

Renderer::Renderer(Window& window_, Device& device_, SceneInfo& scene_info_, std::string state_path) :
    window{window_},
    device{device_},
    scene_info{scene_info_},
    world_state{state_path},
    color_images{IMAGE_HISTORY_COUNT, VK_NULL_HANDLE},
    color_image_views{IMAGE_HISTORY_COUNT, VK_NULL_HANDLE}
{
    createSamplers();
    createStateBuffer();
    uploadState();
    createStateDescriptors();
    createSubchunkStateBuffer();
    zeroSubchunkState();
    createSubchunkStateDescriptors();
    createSceneInfoBuffer();
    createSceneInfoDescriptors();
    recreateSwapchain();
    createSceneInfo(swap_chain->getSwapChainExtent());
    createPipelines();
    createCommandBuffers();
}

Renderer::~Renderer() {
    vkDestroySampler(device.device(), color_sampler, nullptr);
    vkDestroySampler(device.device(), normal_sampler, nullptr);
    vkDestroySampler(device.device(), position_sampler, nullptr);
    for (int i = 0; i < color_image_views.size(); i++) {
        vkDestroyImageView(device.device(), color_image_views[i], nullptr);
    }
    vkDestroyImageView(device.device(), normal_image_view, nullptr);
    vkDestroyImageView(device.device(), position_image_view, nullptr);
    for (int i = 0; i < color_images.size(); i++) {
        vmaDestroyImage(device.allocator(), color_images[i], color_allocations[i]);
    }
    vmaDestroyImage(device.allocator(), normal_image, normal_allocation);
    vmaDestroyImage(device.allocator(), position_image, position_allocation);
    vmaDestroyBuffer(device.allocator(), subchunk_state_buffer, subchunk_state_allocation);
    vmaDestroyBuffer(device.allocator(), state_buffer, state_allocation);
    vmaDestroyBuffer(device.allocator(), scene_info_buffer, scene_info_allocation);
    freeCommandBuffers();
}

void Renderer::createSamplers() {
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 16.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; 
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    vkCreateSampler(device.device(), &sampler_info, nullptr, &color_sampler);
    vkCreateSampler(device.device(), &sampler_info, nullptr, &normal_sampler);
    vkCreateSampler(device.device(), &sampler_info, nullptr, &position_sampler);
}

void Renderer::createStateBuffer() {
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = world_state.getSize();
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocation_info{};
    allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
    allocation_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocation_info.priority = 1.0f;

    vmaCreateBuffer(device.allocator(), &buffer_create_info, &allocation_info, &state_buffer, &state_allocation, nullptr);
}

void Renderer::createSubchunkStateBuffer() {
    int subchunk_size = scene_info.chunk_size / 2;
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = world_state.getSize() / (subchunk_size * subchunk_size * subchunk_size);
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocation_info{};
    allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
    allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocation_info.priority = 1.0f;

    vmaCreateBuffer(device.allocator(), &buffer_create_info, &allocation_info, &subchunk_state_buffer, &subchunk_state_allocation, nullptr);
}

void Renderer::zeroSubchunkState() {
    VkBuffer staging_buffer;
    VmaAllocation staging_allocation;

    int subchunk_size = scene_info.chunk_size / 2;
    int buffer_size = world_state.getSize() / (subchunk_size * subchunk_size * subchunk_size);
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = buffer_size;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocation_info{};
    allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
    allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocation_info.priority = 1.0f;

    VmaAllocationInfo alloc_info;

    vmaCreateBuffer(device.allocator(), &buffer_create_info, &allocation_info, &staging_buffer, &staging_allocation, &alloc_info);

    memset(alloc_info.pMappedData, 1, (size_t)buffer_size);
    copyBuffer(staging_buffer, subchunk_state_buffer, buffer_size);

    vmaDestroyBuffer(device.allocator(), staging_buffer, staging_allocation);
}

void Renderer::uploadState() {
    VkBuffer staging_buffer;
    VmaAllocation staging_allocation;

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = world_state.getSize();
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocation_info{};
    allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
    allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocation_info.priority = 1.0f;

    VmaAllocationInfo alloc_info;

    vmaCreateBuffer(device.allocator(), &buffer_create_info, &allocation_info, &staging_buffer, &staging_allocation, &alloc_info);

    memcpy(alloc_info.pMappedData, world_state.data.data(), (size_t)world_state.getSize());
    copyBuffer(staging_buffer, state_buffer, world_state.getSize());

    vmaDestroyBuffer(device.allocator(), staging_buffer, staging_allocation);
}

void Renderer::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = device.getCommandPool();
    alloc_info.commandBufferCount = static_cast<uint32_t>(1);

    VkCommandBuffer command_buffer;
    if (vkAllocateCommandBuffers(device.device(), &alloc_info, &command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate copy command buffer!");
    }

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(device.computeQueue(), 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.computeQueue());
    vkFreeCommandBuffers(device.device(), device.getCommandPool(), 1, &command_buffer);
}

void Renderer::createStateDescriptors() {
    state_set_layout = DescriptorSetLayout::Builder(device)
    .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
    .build();

    state_pool = DescriptorPool::Builder(device)
    .setMaxSets(1)
    .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1)
    .build();

    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = state_buffer;
    buffer_info.offset = 0;
    buffer_info.range = VK_WHOLE_SIZE;

    DescriptorWriter(*state_set_layout, *state_pool)
    .writeBuffer(0, &buffer_info)
    .build(state_descriptor_set);
}

void Renderer::createSubchunkStateDescriptors() {
    subchunk_state_set_layout = DescriptorSetLayout::Builder(device)
    .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
    .build();

    subchunk_state_pool = DescriptorPool::Builder(device)
    .setMaxSets(1)
    .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1)
    .build();

    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = subchunk_state_buffer;
    buffer_info.offset = 0;
    buffer_info.range = VK_WHOLE_SIZE;

    DescriptorWriter(*subchunk_state_set_layout, *subchunk_state_pool)
    .writeBuffer(0, &buffer_info)
    .build(subchunk_state_descriptor_set);
}

void Renderer::createSceneInfoBuffer() {
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = sizeof(SceneInfo);
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo allocation_info{};
    allocation_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    allocation_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    vmaCreateBuffer(device.allocator(), &buffer_create_info, &allocation_info, &scene_info_buffer, &scene_info_allocation, &scene_info_mapped);
}

void Renderer::createSceneInfoDescriptors() {
    scene_info_set_layout = DescriptorSetLayout::Builder(device)
    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
    .build();

    scene_info_pool = DescriptorPool::Builder(device)
    .setMaxSets(1)
    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
    .build();

    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = scene_info_buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(SceneInfo);

    DescriptorWriter(*scene_info_set_layout, *scene_info_pool)
    .writeBuffer(0, &buffer_info)
    .build(scene_info_descriptor_set);
}

void Renderer::createSceneInfo(VkExtent2D extent) {
    glm::ivec3 world_dimensions = world_state.getDimensions();
    scene_info.screen_dimensions = glm::ivec2{extent.width, extent.height};
    scene_info.world_dimensions = world_dimensions;
    scene_info.camera_position = glm::vec3{world_dimensions.x / 2, world_dimensions.y / 2, -12.0f};
    scene_info.camera_direction = glm::vec3{0.0, 0.0, 1.0};
    scene_info.old_camera_position = scene_info.camera_position;
    scene_info.old_camera_direction = scene_info.camera_direction;

    memcpy(scene_info_mapped.pMappedData, &scene_info, sizeof(scene_info));
}

void Renderer::recreateSceneInfo(VkExtent2D extent) {
    scene_info.screen_dimensions = glm::ivec2{extent.width, extent.height};

    memcpy(scene_info_mapped.pMappedData, &scene_info, sizeof(scene_info));
}

void Renderer::updateSceneInfo() {
    memcpy(scene_info_mapped.pMappedData, &scene_info, sizeof(scene_info));
}

void Renderer::recreateDenoiseImages(VkExtent2D extent) {
    SwapChainSupportDetails swapchain_support = device.getSwapChainSupport();
    VkSurfaceFormatKHR surface_format;
    for (auto& format : swapchain_support.formats) {
        if (device.checkFormatSupport(format.format, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
            surface_format.format = format.format;
            break;
        }
    }

    color_images.resize(IMAGE_HISTORY_COUNT);
    color_allocations.resize(IMAGE_HISTORY_COUNT);
    for (int i = 0; i < color_images.size(); i++) {
        if (color_images[i] != VK_NULL_HANDLE) { vmaDestroyImage(device.allocator(), color_images[i], color_allocations[i]); }
    }
    if (normal_image != VK_NULL_HANDLE) { vmaDestroyImage(device.allocator(), normal_image, normal_allocation); }
    if (position_image != VK_NULL_HANDLE) { vmaDestroyImage(device.allocator(), position_image, position_allocation); }

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = extent.width;
    image_create_info.extent.height = extent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = surface_format.format;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_STORAGE_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.flags = 0;

    VmaAllocationCreateInfo allocation_info{};
    allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
    allocation_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocation_info.priority = 1.0f;

    for (int i = 0; i < color_images.size(); i++) {
        vmaCreateImage(device.allocator(), &image_create_info, &allocation_info, &color_images[i], &color_allocations[i], nullptr);
    }
    vmaCreateImage(device.allocator(), &image_create_info, &allocation_info, &normal_image, &normal_allocation, nullptr);
    vmaCreateImage(device.allocator(), &image_create_info, &allocation_info, &position_image, &position_allocation, nullptr);

    for (int i = 0; i < color_image_views.size(); i++) {
        if (color_image_views[i] != VK_NULL_HANDLE) { vkDestroyImageView(device.device(), color_image_views[i], nullptr); }
    }
    if (normal_image_view != VK_NULL_HANDLE) { vkDestroyImageView(device.device(), normal_image_view, nullptr); }
    if (position_image_view != VK_NULL_HANDLE) { vkDestroyImageView(device.device(), position_image_view, nullptr); }

    VkImageViewCreateInfo imview_create_info{};
    imview_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imview_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imview_create_info.format = surface_format.format;
    imview_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imview_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imview_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imview_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imview_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imview_create_info.subresourceRange.baseMipLevel = 0;
    imview_create_info.subresourceRange.levelCount = 1;
    imview_create_info.subresourceRange.baseArrayLayer = 0;
    imview_create_info.subresourceRange.layerCount = 1;

    for (int i = 0; i < color_image_views.size(); i++) {
        imview_create_info.image = color_images[i];
        if (vkCreateImageView(device.device(), &imview_create_info, nullptr, &color_image_views[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create color image view!");
        }
    }

    imview_create_info.image = normal_image;
    if (vkCreateImageView(device.device(), &imview_create_info, nullptr, &normal_image_view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create normal image view!");
    }

    imview_create_info.image = position_image;
    if (vkCreateImageView(device.device(), &imview_create_info, nullptr, &position_image_view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create position image view!");
    }
}

void Renderer::recreateSwapchain() {
    auto extent = window.getExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device.device());

    if (swap_chain == nullptr) {
        swap_chain = std::make_unique<SwapChain>(device, extent);
    } else {
        std::shared_ptr<SwapChain> old_swap_chain = std::move(swap_chain);
        swap_chain = std::make_unique<SwapChain>(device, extent, old_swap_chain);

        if (!old_swap_chain->compareSwapFormats(*swap_chain.get())) {
            throw std::runtime_error("Swap chain image or depth format has changed!");
        }
    }

    recreateDenoiseImages(extent);

    createFrameDescriptors();
    recreateSceneInfo(swap_chain->getSwapChainExtent());
}

void Renderer::createFrameDescriptors() {
    // Create color image descriptors
    frame_set_layout = DescriptorSetLayout::Builder(device)
    .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1, &color_sampler)
    .build();

    frame_pool = DescriptorPool::Builder(device)
    .setMaxSets(IMAGE_HISTORY_COUNT + swap_chain->imageCount())
    .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, IMAGE_HISTORY_COUNT + swap_chain->imageCount())
    .build();

    color_descriptor_sets.resize(IMAGE_HISTORY_COUNT);
    for (int i = 0; i < color_descriptor_sets.size(); i++) {
        VkDescriptorImageInfo color_info{};
        color_info.imageView = color_image_views[i];
        color_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        color_info.sampler = color_sampler;

        DescriptorWriter(*frame_set_layout, *frame_pool)
        .writeImage(0, &color_info)
        .build(color_descriptor_sets[i]);
    }

    present_descriptor_sets.resize(swap_chain->imageCount());
    for (int i = 0; i < present_descriptor_sets.size(); i++) {
        VkDescriptorImageInfo present_info{};
        present_info.imageView = swap_chain->getImageView(i);
        present_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        present_info.sampler = color_sampler;

        DescriptorWriter(*frame_set_layout, *frame_pool)
        .writeImage(0, &present_info)
        .build(present_descriptor_sets[i]);
    }

    // Create normal image descriptors
    normal_set_layout = DescriptorSetLayout::Builder(device)
    .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1, &normal_sampler)
    .build();

    normal_pool = DescriptorPool::Builder(device)
    .setMaxSets(swap_chain->imageCount())
    .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, swap_chain->imageCount())
    .build();

    VkDescriptorImageInfo normal_info{};
    normal_info.imageView = normal_image_view;
    normal_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    normal_info.sampler = normal_sampler;

    DescriptorWriter(*normal_set_layout, *normal_pool)
    .writeImage(0, &normal_info)
    .build(normal_descriptor_set);

    // Create position image descriptors
    position_set_layout = DescriptorSetLayout::Builder(device)
    .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1, &position_sampler)
    .build();

    position_pool = DescriptorPool::Builder(device)
    .setMaxSets(swap_chain->imageCount())
    .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, swap_chain->imageCount())
    .build();

    VkDescriptorImageInfo position_info{};
    position_info.imageView = position_image_view;
    position_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    position_info.sampler = position_sampler;

    DescriptorWriter(*position_set_layout, *position_pool)
    .writeImage(0, &position_info)
    .build(position_descriptor_set);
}

void Renderer::createPipelines() {
    // Create physics pipeline
    VkPushConstantRange subc_push_const_range{};
    subc_push_const_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    subc_push_const_range.offset = 0;
    subc_push_const_range.size = sizeof(PhysicsPushConstant);
    std::vector<VkPushConstantRange> subc_push_const_ranges = { subc_push_const_range };

    std::vector<VkDescriptorSetLayout> physics_set_layouts = { state_set_layout->getDescriptorSetLayout(), scene_info_set_layout->getDescriptorSetLayout(), subchunk_state_set_layout->getDescriptorSetLayout() };
    physics_pipeline = std::make_unique<Pipeline>(device, shader_dir + "physics.comp.spv", physics_set_layouts, subc_push_const_ranges);

    // Create graphics pipeline
    VkPushConstantRange rt_push_const_range{};
    rt_push_const_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    rt_push_const_range.offset = 0;
    rt_push_const_range.size = sizeof(RaytraceSettingsPushConstant);
    std::vector<VkPushConstantRange> rt_push_const_ranges = { rt_push_const_range };
    
    std::vector<VkDescriptorSetLayout> graphics_set_layouts = { frame_set_layout->getDescriptorSetLayout(), frame_set_layout->getDescriptorSetLayout(), normal_set_layout->getDescriptorSetLayout(), position_set_layout->getDescriptorSetLayout(), state_set_layout->getDescriptorSetLayout(), scene_info_set_layout->getDescriptorSetLayout(), subchunk_state_set_layout->getDescriptorSetLayout() };
    graphics_pipeline = std::make_unique<Pipeline>(device, shader_dir + "raytrace.comp.spv", graphics_set_layouts, rt_push_const_ranges);

    // Create postprocessing pipeline
    VkPushConstantRange postp_push_const_range{};
    postp_push_const_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    postp_push_const_range.offset = 0;
    postp_push_const_range.size = sizeof(PostProcessingPushConstant);
    std::vector<VkPushConstantRange> postp_push_const_ranges = { postp_push_const_range };

    std::vector<VkDescriptorSetLayout> postprocess_set_layouts = { frame_set_layout->getDescriptorSetLayout(), frame_set_layout->getDescriptorSetLayout(), frame_set_layout->getDescriptorSetLayout(), normal_set_layout->getDescriptorSetLayout(), position_set_layout->getDescriptorSetLayout() };
    postprocess_pipeline = std::make_unique<Pipeline>(device, shader_dir + "postprocess.comp.spv", postprocess_set_layouts, postp_push_const_ranges);
}

void Renderer::createCommandBuffers() {
    command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = device.getCommandPool();
    alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

    if (vkAllocateCommandBuffers(device.device(), &alloc_info, command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void Renderer::freeCommandBuffers() {
    vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
    command_buffers.clear();
}

VkCommandBuffer Renderer::beginFrame() {
    if (is_frame_started) {
        throw std::runtime_error("Can't call beginFrame while frame already in progress");
    }

    render_settings.frame_num++;

    if (reset_accumulation) {
        reset_accumulation = false;
        render_settings.invalidate_accumulation = false;
    }

    if (invalidate_accumulation) {
        invalidate_accumulation = false;
        render_settings.invalidate_accumulation = true;
        reset_accumulation = true;
    }

    prev_image_index = ((render_settings.frame_num - 1) % IMAGE_HISTORY_COUNT);
    curr_image_index = ((render_settings.frame_num) % IMAGE_HISTORY_COUNT);
    auto result = swap_chain->acquireNextImage(&submit_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    is_frame_started = true;

    auto command_buffer = getCurrentCommandBuffer();

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    swap_chain->recordImageBarrier(command_buffer, swap_chain->getImage(submit_image_index),
                                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                                   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    for (int i = 0; i < color_images.size(); i++) {
        swap_chain->recordImageBarrier(command_buffer, color_images[i],
                                       VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                                       VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                                       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }

    swap_chain->recordImageBarrier(command_buffer, normal_image,
                                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                                   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    swap_chain->recordImageBarrier(command_buffer, position_image,
                                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                                   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    return command_buffer;
}

void Renderer::endFrame() {
    if (!is_frame_started) {
        throw std::runtime_error("Can't call endFrame while frame not in progress!");   
    }
    auto command_buffer = getCurrentCommandBuffer();

    swap_chain->recordImageBarrier(command_buffer, swap_chain->getImage(submit_image_index),
                                   VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                   VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
                                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    auto result = swap_chain->submitCommandBuffers(&command_buffer, &submit_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
        window.resetWindowResizedFlag();
        recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    is_frame_started = false;
    curr_frame_index = 0;
}

void Renderer::render() {
    updateSceneInfo();

    auto command_buffer = getCurrentCommandBuffer();

    /*  Create physics structures */
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, physics_pipeline->getPipeline());
    std::vector<VkDescriptorSet> physics_descriptor_sets;
    physics_descriptor_sets.push_back(state_descriptor_set);
    physics_descriptor_sets.push_back(scene_info_descriptor_set);
    physics_descriptor_sets.push_back(subchunk_state_descriptor_set);
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, physics_pipeline->getPipelineLayout(), 0, physics_descriptor_sets.size(), physics_descriptor_sets.data(), 0, nullptr);

    if (world_state.x_size != world_state.y_size || world_state.x_size != world_state.z_size || world_state.y_size != world_state.z_size) {
        throw std::runtime_error("only worlds with square dimensions are supported!");
    }
    int group_count_x = (world_state.x_size / (scene_info.chunk_size * scene_info.local_size)) + 1;
    int group_count_y = (world_state.y_size / (scene_info.chunk_size * scene_info.local_size)) + 1;
    int group_count_z = (world_state.z_size / (scene_info.chunk_size * scene_info.local_size)) + 1;

    /*  Create barrier structures   */
    VkMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR;
    barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT_KHR;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT_KHR;

    /*  Evolve physical system  */
    VkDependencyInfoKHR dep_info{};
    dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
    dep_info.memoryBarrierCount = 1;
    dep_info.pMemoryBarriers = &barrier;

    for (int i = 0; i < 8; i++) {
        int subchunk_size = scene_info.chunk_size / 2;
        switch (i) {
        case 0:
            physics_settings.subchunk_location = glm::ivec3(0, 0, 0);
            break;
        case 1:
            physics_settings.subchunk_location = glm::ivec3(1, 0, 0);
            break;
        case 2:
            physics_settings.subchunk_location = glm::ivec3(0, 0, 1);
            break;
        case 3:
            physics_settings.subchunk_location = glm::ivec3(1, 0, 1);
            break;
        case 4:
            physics_settings.subchunk_location = glm::ivec3(0, 1, 0);
            break;
        case 5:
            physics_settings.subchunk_location = glm::ivec3(1, 1, 0);
            break;
        case 6:
            physics_settings.subchunk_location = glm::ivec3(0, 1, 1);
            break;
        case 7:
            physics_settings.subchunk_location = glm::ivec3(1, 1, 1);
            break;
        }
        int rand_offset = Rand::range(0, scene_info.chunk_size - 1);
        physics_settings.subchunk_offset = (physics_settings.subchunk_location * subchunk_size) - glm::ivec3(rand_offset, rand_offset, rand_offset);

        vkCmdPushConstants(command_buffer, physics_pipeline->getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PhysicsPushConstant), &physics_settings);
        vkCmdDispatch(command_buffer, group_count_x, group_count_y, group_count_z);
        vkCmdPipelineBarrier2(command_buffer, &dep_info);
    }

    /*  Render world state to image    */
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, graphics_pipeline->getPipeline());
    std::vector<VkDescriptorSet> graphics_descriptor_sets;
    graphics_descriptor_sets.push_back(color_descriptor_sets[curr_image_index]);
    graphics_descriptor_sets.push_back(color_descriptor_sets[prev_image_index]);
    graphics_descriptor_sets.push_back(normal_descriptor_set);
    graphics_descriptor_sets.push_back(position_descriptor_set);
    graphics_descriptor_sets.push_back(state_descriptor_set);
    graphics_descriptor_sets.push_back(scene_info_descriptor_set);
    graphics_descriptor_sets.push_back(subchunk_state_descriptor_set);
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, graphics_pipeline->getPipelineLayout(), 0, graphics_descriptor_sets.size(), graphics_descriptor_sets.data(), 0, nullptr);

    vkCmdPushConstants(command_buffer, graphics_pipeline->getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(RaytraceSettingsPushConstant), &render_settings);
    group_count_x = (swap_chain->width() / 32) + 1;
    group_count_y = (swap_chain->height() / 32) + 1;
    group_count_z = 1;
    vkCmdDispatch(command_buffer, group_count_x, group_count_y, group_count_z);
    vkCmdPipelineBarrier2(command_buffer, &dep_info);

    /*  Denoise rendered image  */
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, postprocess_pipeline->getPipeline());
    std::vector<VkDescriptorSet> postprocess_descriptor_sets;
    postprocess_descriptor_sets.push_back(present_descriptor_sets[submit_image_index]);
    postprocess_descriptor_sets.push_back(color_descriptor_sets[curr_image_index]);
    postprocess_descriptor_sets.push_back(color_descriptor_sets[prev_image_index]);
    postprocess_descriptor_sets.push_back(normal_descriptor_set);
    postprocess_descriptor_sets.push_back(position_descriptor_set);
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, postprocess_pipeline->getPipelineLayout(), 0, postprocess_descriptor_sets.size(), postprocess_descriptor_sets.data(), 0, nullptr);

    for (int i = 0; i < renderer_settings.denoise_iterations; i++) {
        postprocess_settings.denoise_iteration = i;

        vkCmdPushConstants(command_buffer, postprocess_pipeline->getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PostProcessingPushConstant), &postprocess_settings);
        vkCmdDispatch(command_buffer, group_count_x, group_count_y, group_count_z);
        vkCmdPipelineBarrier2(command_buffer, &dep_info);
    }
}

}