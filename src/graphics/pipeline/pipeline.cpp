#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include "pipeline.h"

namespace cscd
{

Pipeline::Pipeline(Device &device_, const std::string &compsh_path, std::vector<VkDescriptorSetLayout>& set_layouts, std::vector<VkPushConstantRange>& push_const_ranges) : device{device_}
{
    createShaderModule(compsh_path, &compsh_module);
    createComputePipelineLayout(set_layouts, push_const_ranges);
    createComputePipeline(compsh_path);
}

Pipeline::~Pipeline()
{
    vkDestroyPipelineLayout(device.device(), compute_pipeline_layout, nullptr);
    vkDestroyShaderModule(device.device(), compsh_module, nullptr);
    vkDestroyPipeline(device.device(), compute_pipeline, nullptr);
}

void Pipeline::createComputePipelineLayout(std::vector<VkDescriptorSetLayout>& set_layouts, std::vector<VkPushConstantRange>& push_const_ranges) {
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
    pipeline_layout_info.pSetLayouts = set_layouts.data();
    pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(push_const_ranges.size());
    if (push_const_ranges.size() != 0) {
        pipeline_layout_info.pPushConstantRanges = push_const_ranges.data();
    }

    if (vkCreatePipelineLayout(device.device(), &pipeline_layout_info, nullptr, &compute_pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }
}

std::vector<char> Pipeline::readShaderFile(const std::string &path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + path);
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();
    return buffer;
}

void Pipeline::createShaderModule(const std::string &path, VkShaderModule *shader_module)
{
    auto shader_code = readShaderFile(path);

    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shader_code.size();
    create_info.pCode = reinterpret_cast<const uint32_t *>(shader_code.data());

    if (vkCreateShaderModule(device.device(), &create_info, nullptr, shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module");
    }
}

void Pipeline::createComputePipeline(const std::string &compsh_path)
{
    if (compsh_module == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to create compute pipeline, shader module has not been created!");
    }

    VkPipelineShaderStageCreateInfo shader_stage;
    shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shader_stage.module = compsh_module;
    shader_stage.pName = "main";
    shader_stage.flags = 0;
    shader_stage.pNext = nullptr;
    shader_stage.pSpecializationInfo = nullptr;

    if (compute_pipeline_layout == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to create compute pipeline, layout has not been created!");
    }

    VkComputePipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.layout = compute_pipeline_layout;
    pipeline_info.stage = shader_stage;

    if (vkCreateComputePipelines(device.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &compute_pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline!");
    }
}

}