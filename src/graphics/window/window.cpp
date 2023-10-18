#include <stdexcept>
#include "window.h"

namespace cscd {

Window::Window(int width_, int height_, std::string name) : width{width_}, height{height_}, window_name{name} {
    initWindow();
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void Window::framebufferResizedCallback(GLFWwindow *glfw_window, int width, int height) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window->framebuffer_resized = true;
    window->width = width;
    window->height = height;
}

}