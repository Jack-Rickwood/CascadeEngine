#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

namespace cscd {

struct SceneInfo {
    glm::ivec2 screen_dimensions;
    alignas(16) glm::ivec3 world_dimensions;

    alignas(16) glm::vec3 camera_position;
    alignas(16) glm::vec3 camera_direction;
    alignas(16) glm::vec3 old_camera_position;
    alignas(16) glm::vec3 old_camera_direction;

    alignas(4) int chunk_size = 16;
    alignas(4) int local_size = 8;
};

struct RendererSettings {
    int denoise_iterations = 3;
};

struct RaytraceSettingsPushConstant {
    int frame_num = 0;
    alignas(4) int max_ray_steps = 128;
    alignas(4) int max_bounces = 3;
    alignas(4) int rays_per_pixel = 4;
    alignas(4) int use_blue_noise = true; // int to avoid weird alignment issues
    alignas(4) int use_temp_accumulation = false; // int to avoid weird alignment issues
    alignas(4) int invalidate_accumulation = false; // int to avoid weird alignment issues
};

struct PhysicsPushConstant {
    glm::ivec3 subchunk_offset;
    alignas(16) glm::ivec3 subchunk_location;
};

struct PostProcessingPushConstant {
    int use_smart_denoise = false;  // int to avoid weird alignment issues
    alignas(4) int use_atrous_denoise = false; // int to avoid weird alignment issues
    alignas(4) int denoise_iteration = 0;
    alignas(4) float c_phi = 0.01f;
    alignas(4) float n_phi = 0.005f;
    alignas(4) float p_phi = 0.3f;
};

}