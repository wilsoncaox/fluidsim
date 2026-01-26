#pragma once

#include "Window.hpp"

#include <optional>
#include <vector>


struct QueueIndices {
  std::optional<uint32_t> index;

  bool is_complete() {
    return index.has_value();
  }
};

class VulkanContext {
   
  public:
    ~VulkanContext();

    void init(Window& window);
    VkPhysicalDeviceLimits find_device_limit();

    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    QueueIndices queue_index;
    VkQueue queue = VK_NULL_HANDLE;

    bool active = false;

  private:
    void init_vulkan();

    void init_debugger();
    void init_surface(Window& window);

    void pick_physical_device();
    bool is_device_suitable(VkPhysicalDevice device);

    QueueIndices find_queue_family(VkPhysicalDevice device);

    void init_device();

    void reset();

    bool check_validation_support() const;
    bool check_extension_support(VkPhysicalDevice device) const;
    
    std::vector<const char*> get_required_extensions() const;

    VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance, 
      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
      const VkAllocationCallbacks* pAllocator, 
      VkDebugUtilsMessengerEXT* pDebugMessenger
    );

    void DestroyDebugUtilsMessengerEXT(
      VkInstance instance, 
      VkDebugUtilsMessengerEXT debugMessenger, 
      const VkAllocationCallbacks* pAllocator
    );

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData
    );

    VkInstance instance = VK_NULL_HANDLE;

    bool enabledValidationLayers = true;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    const std::vector<const char*> layers { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> device_extensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };



};
