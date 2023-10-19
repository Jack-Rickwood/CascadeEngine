#pragma once

#include <string>
#include <vector>
#include <optional>
#include "externals/VMA/vk_mem_alloc.h"
#include "graphics/window/window.h"

// #define DEBUG_MODE

namespace cscd {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> compute_family;
    std::optional<uint32_t> present_family;
    bool isComplete() { return compute_family.has_value() && present_family.has_value(); }
};

class Device {
public:
#ifdef DEBUG_MODE
    static constexpr bool enable_validation_layers = true;
#else
    static constexpr bool enable_validation_layers = false;
#endif

    Device(Window &window);
    ~Device();

    // Not copyable or movable
    Device(const Device &) = delete;
    Device& operator=(const Device &) = delete;
    Device(Device &&) = delete;
    Device& operator=(Device &&) = delete;

    VkCommandPool getCommandPool() { return command_pool; }
    VmaAllocator allocator() { return allocator_; }
    VkDevice device() { return device_; }
    VkSurfaceKHR surface() { return surface_; }
    VkQueue computeQueue() { return compute_queue_; }
    VkQueue presentQueue() { return present_queue_; }

    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physical_device); }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physical_device); }
    bool checkFormatSupport(VkFormat format, VkFormatFeatureFlags requestedSupport);

    void createImageWithInfo(
        const VkImageCreateInfo &imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory
    );

    VkPhysicalDeviceProperties properties;

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createAllocator();
    void createCommandPool();

    // Helper functions
    bool isDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char *> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info);
    void hasGlfwRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VmaAllocator allocator_;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    Window& window;
    VkCommandPool command_pool;

    VkDevice device_;
    VkSurfaceKHR surface_;
    VkQueue compute_queue_;
    VkQueue present_queue_;

    const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

}