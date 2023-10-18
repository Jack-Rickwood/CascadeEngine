#pragma once
#include "settings/settings.h"
#include "graphics/window/window.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

namespace cscd {

class KeyboardInputController {
public:
    struct KeyMapping {
        int move_up = GLFW_KEY_E;
        int move_down = GLFW_KEY_Q;
        int move_left = GLFW_KEY_A;
        int move_right = GLFW_KEY_D;
        int move_forward = GLFW_KEY_W;
        int move_backward = GLFW_KEY_S;
        int look_up = GLFW_KEY_UP;
        int look_down = GLFW_KEY_DOWN;
        int look_left = GLFW_KEY_LEFT;
        int look_right = GLFW_KEY_RIGHT;
    };

    struct InputSettings {
        float move_speed = 30.0f;
        float look_speed = 1.0f;
    };

    void update(GLFWwindow* window, float dt, SceneInfo& scene_info);

    KeyMapping keys{};
    InputSettings settings{};
};

}