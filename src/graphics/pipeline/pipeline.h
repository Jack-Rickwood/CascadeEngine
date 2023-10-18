#pragma once

#include <string>
#include <vector>
#include "graphics/device/device.h"

namespace cscd {

class Pipeline {
public:
    Pipeline(Device& device_, const std::string& compsh_path, std::vector<VkDescriptorSetLayout>& set_layouts, std::vector<VkPushConstantRange>& push_const_ranges);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    VkPipelineLayout& getPipelineLayout() { return compute_pipeline_layout; }
    VkPipeline& getPipeline() { return compute_pipeline; }

private:
    static std::vector<char> readShaderFile(const std::string& path);
    void createShaderModule(const std::string& path, VkShaderModule *shader_module);
    void createComputePipelineLayout(std::vector<VkDescriptorSetLayout>& set_layouts, std::vector<VkPushConstantRange>& push_const_ranges);
    void createComputePipeline(const std::string& compsh_path);

    Device& device;
    VkPipelineLayout compute_pipeline_layout;
    VkShaderModule compsh_module;
    VkPipeline compute_pipeline;
};

}