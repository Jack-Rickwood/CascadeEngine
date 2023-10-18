#include <limits>
#include "keyboard_input_controller.h"

namespace cscd {

void KeyboardInputController::update(GLFWwindow* window, float dt, SceneInfo& scene_info) {
    scene_info.old_camera_direction = scene_info.camera_direction;
    scene_info.old_camera_position = scene_info.camera_position;

    glm::vec2 look{
        atan2(scene_info.camera_direction.z, scene_info.camera_direction.x),
        asin(scene_info.camera_direction.y)
    };
    
    float look_amount = glm::pi<float>() * settings.look_speed * dt;
    if (glfwGetKey(window, keys.look_up) == GLFW_PRESS) look.y += look_amount;
    if (glfwGetKey(window, keys.look_down) == GLFW_PRESS) look.y -= look_amount;
    if (glfwGetKey(window, keys.look_left) == GLFW_PRESS) look.x -= look_amount;
    if (glfwGetKey(window, keys.look_right) == GLFW_PRESS) look.x += look_amount;

    look.x = glm::mod(look.x + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
    look.y = glm::clamp(look.y, -glm::half_pi<float>(), glm::half_pi<float>());

    scene_info.camera_direction.x = cos(look.x) * cos(look.y);
    scene_info.camera_direction.z = sin(look.x) * cos(look.y);
    scene_info.camera_direction.y = sin(look.y);

    scene_info.camera_direction = glm::normalize(scene_info.camera_direction);
    //printf("x: %f\ty: %f\tz: %f\n", scene_info.camera_direction.x, scene_info.camera_direction.y, scene_info.camera_direction.z);

    float move_amount = 1.0f * settings.move_speed * dt;
    glm::vec3 camera_direction_plane = glm::normalize(scene_info.camera_direction * glm::vec3(1.0f, 0.0f, 1.0f));
    glm::mat3x3 rot_x = glm::mat3x3(1.0f, 0.0f, 0.0f, 0.0f, cos(glm::half_pi<float>()), -sin(glm::half_pi<float>()), 0.0f, sin(glm::half_pi<float>()), cos(glm::half_pi<float>()));
    glm::mat3x3 rot_y = glm::mat3x3(cos(glm::half_pi<float>()), 0.0f, sin(glm::half_pi<float>()), 0.0f, 1.0f, 0.0f, -sin(glm::half_pi<float>()), 0.0f, cos(glm::half_pi<float>()));

    if (glfwGetKey(window, keys.move_up) == GLFW_PRESS) scene_info.camera_position.y += move_amount;
    if (glfwGetKey(window, keys.move_down) == GLFW_PRESS) scene_info.camera_position.y -= move_amount;
    if (glfwGetKey(window, keys.move_left) == GLFW_PRESS) scene_info.camera_position += move_amount * camera_direction_plane * rot_y;
    if (glfwGetKey(window, keys.move_right) == GLFW_PRESS) scene_info.camera_position -= move_amount * camera_direction_plane * rot_y;
    if (glfwGetKey(window, keys.move_forward) == GLFW_PRESS) scene_info.camera_position += move_amount * camera_direction_plane;
    if (glfwGetKey(window, keys.move_backward) == GLFW_PRESS) scene_info.camera_position -= move_amount * camera_direction_plane;
}

}