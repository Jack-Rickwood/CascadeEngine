#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <stdexcept>
#include <array>
#include <chrono>
#include <iostream>
#include <thread>
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "application.h"

namespace cscd {

Application::Application(std::string state_path) :
    renderer{window, device, scene_info, state_path}
{}

Application::~Application() {}

void Application::run() {
    KeyboardInputController camera_controller{};

    std::thread config_thread([this]() {
        configWindow();
    });
    
    auto curr_time = std::chrono::high_resolution_clock::now();
    while (!window.shouldClose()) {
        glfwPollEvents();

        auto new_time = std::chrono::high_resolution_clock::now();
        float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - curr_time).count();
        curr_time = new_time;
        std::cout << std::to_string(1.0f / frame_time) << std::endl;

        camera_controller.update(window.getGLFWWindow(), frame_time, scene_info);

        //printf("x: %f\ty: %f\tz: %f\n", scene_info.camera_position.x, scene_info.camera_position.y, scene_info.camera_position.z);
        //printf("x: %f\ty: %f\tz: %f\n\n\n", scene_info.camera_direction.x, scene_info.camera_direction.y, scene_info.camera_direction.z);
        
        if (auto command_buffer = renderer.beginFrame()) {
            renderer.render();
            renderer.endFrame();
        }
    }

    vkDeviceWaitIdle(device.device());
    //stop_config_ui = true;
    config_thread.join();
}

}