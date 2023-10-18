#pragma once
#define GLFW_INCLUDE_VULKAN

#include <string>
#include <GLFW/glfw3.h>

namespace cscd {

class Window {
public:
    Window(int width_, int height_, std::string name);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() { return glfwWindowShouldClose(window); }
    VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
    bool wasWindowResized() { return framebuffer_resized; }
    void resetWindowResizedFlag() { framebuffer_resized = false; }
    GLFWwindow* getGLFWWindow() const { return window; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

private:
    static void framebufferResizedCallback(GLFWwindow *glfw_window, int width, int height);
    void initWindow();
    
    int width;
    int height;
    bool framebuffer_resized;
    
    std::string window_name;
    GLFWwindow *window;
};

}