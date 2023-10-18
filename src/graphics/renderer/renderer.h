#pragma once

#include <memory>
#include <vector>
#include <cassert>
#include "graphics/window/window.h"
#include "graphics/device/device.h"
#include "graphics/swap_chain/swap_chain.h"
#include "graphics/pipeline/pipeline.h"
#include "graphics/descriptors/descriptors.h"
#include "files/state_file.h"
#include "settings/settings.h"

#define IMAGE_HISTORY_COUNT 2
#define SWAPCHAIN_IMAGES 1
#define TOTAL_COLOR_IMAGES (IMAGE_HISTORY_COUNT + SWAPCHAIN_IMAGES)

namespace cscd {

class Renderer {
public:
#ifdef DEBUG_MODE
    const std::string shader_dir = "src/graphics/shaders/";
#else
    const std::string shader_dir = "shaders/";
#endif

    Renderer(Window& window_, Device& device_, SceneInfo& scene_info_, std::string state_path);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    float getAspectRatio() const { return swap_chain->extentAspectRatio(); }
    bool isFrameInProgress() const { return is_frame_started; }

    VkCommandBuffer getCurrentCommandBuffer() const {
        if (!is_frame_started) {
            throw std::runtime_error("Cannot get command buffer when frame not in progress!");
        }
        return command_buffers[curr_frame_index];
    }

    int getFrameIndex() const {
        if (!is_frame_started) {
            throw std::runtime_error("Cannot get frame index when frame not in progress!");
        }
        return curr_frame_index;
    }

    void copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size); // Abstract into class at some point

    VkCommandBuffer beginFrame();
    void render();
    void endFrame();

    PostProcessingPushConstant& getPostprocessSettings() {
        return postprocess_settings;
    }

    RendererSettings& getRendererSettings() {
        return renderer_settings;
    }

    RaytraceSettingsPushConstant& getRaytraceSettings() {
        return render_settings;
    }

    void invalidateAccumulatedFrames() {
        invalidate_accumulation = true;
    }

private:
    void createSamplers();
    void createStateBuffer();
    void uploadState();
    void createStateDescriptors();
    void createSubchunkStateBuffer();
    void createSubchunkStateDescriptors();
    void createSceneInfoBuffer();
    void createSceneInfoDescriptors();
    void createCommandBuffers();
    void freeCommandBuffers();
    void createSceneInfo(VkExtent2D extent);
    void recreateSceneInfo(VkExtent2D extent);
    void updateSceneInfo();
    void recreateDenoiseImages(VkExtent2D extent);
    void recreateSwapchain();
    void createFrameDescriptors();
    void createPipelines();

    SceneInfo& scene_info;
    PhysicsPushConstant physics_settings{};
    RaytraceSettingsPushConstant render_settings{};
    PostProcessingPushConstant postprocess_settings{};
    RendererSettings renderer_settings{};

    Window& window;
    Device& device;
    std::unique_ptr<SwapChain> swap_chain;
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkImage> color_images;
    VkImage normal_image = VK_NULL_HANDLE;
    VkImage position_image = VK_NULL_HANDLE;
    std::vector<VmaAllocation> color_allocations;
    VmaAllocation normal_allocation;
    VmaAllocation position_allocation;
    std::vector<VkImageView> color_image_views;
    VkImageView normal_image_view = VK_NULL_HANDLE;
    VkImageView position_image_view = VK_NULL_HANDLE;
    VkSampler color_sampler;
    VkSampler normal_sampler;
    VkSampler position_sampler;

    VkBuffer scene_info_buffer;
    VmaAllocation scene_info_allocation;
    VmaAllocationInfo scene_info_mapped;

    file::State world_state;
    VkBuffer state_buffer;
    VmaAllocation state_allocation;
    VkBuffer subchunk_state_buffer;
    VmaAllocation subchunk_state_allocation;

    uint32_t prev_image_index{0};
    uint32_t curr_image_index{0};
    uint32_t submit_image_index{0};
    int curr_frame_index{0};
    bool is_frame_started{false};

    bool invalidate_accumulation = false;
    bool reset_accumulation = false;

    std::unique_ptr<Pipeline> graphics_pipeline;
    std::unique_ptr<Pipeline> physics_pipeline;
    std::unique_ptr<Pipeline> postprocess_pipeline;

    std::unique_ptr<DescriptorPool> frame_pool{};
    std::unique_ptr<DescriptorPool> normal_pool{};
    std::unique_ptr<DescriptorPool> position_pool{};
    std::unique_ptr<DescriptorPool> state_pool{};
    std::unique_ptr<DescriptorPool> subchunk_state_pool{};
    std::unique_ptr<DescriptorPool> scene_info_pool{};
    std::unique_ptr<DescriptorSetLayout> frame_set_layout{};
    std::unique_ptr<DescriptorSetLayout> normal_set_layout{};
    std::unique_ptr<DescriptorSetLayout> position_set_layout{};
    std::unique_ptr<DescriptorSetLayout> state_set_layout{};
    std::unique_ptr<DescriptorSetLayout> subchunk_state_set_layout{};
    std::unique_ptr<DescriptorSetLayout> scene_info_set_layout{};
    std::vector<VkDescriptorSet> present_descriptor_sets;
    std::vector<VkDescriptorSet> color_descriptor_sets;
    VkDescriptorSet normal_descriptor_set;
    VkDescriptorSet position_descriptor_set;
    VkDescriptorSet state_descriptor_set;
    VkDescriptorSet subchunk_state_descriptor_set;
    VkDescriptorSet scene_info_descriptor_set;
};

}