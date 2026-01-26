
#include "VulkanContext.hpp"

#include <iostream>
#include <cstring>
#include <set>

#include <iostream>
#include <vulkan/vulkan_core.h>

VulkanContext::~VulkanContext() {
  reset();
}

void VulkanContext::reset() {
  if (enabledValidationLayers) {
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  }

  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
}

void VulkanContext::init(Window& window) {
  if (!active) {
    active = true;

    init_vulkan();
    init_debugger();
    init_surface(window);
    pick_physical_device();

    init_device();
  } else {
    reset();
    active = false;
    init(window);
  }
}

void VulkanContext::init_vulkan() {
  if (enabledValidationLayers && !check_validation_support()) {
    throw std::runtime_error("Validation layer requested, but unavailable");
  }

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "3D Particle Physics Render";
  app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;
  app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  app_info.pEngineName = "Engine";

  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;

  std::vector<const char*> extensions = get_required_extensions();
  instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  instance_info.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debug_info{};
  if (enabledValidationLayers) {

    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_info.pfnUserCallback = debugCallback;

    instance_info.enabledLayerCount = static_cast<uint32_t>(layers.size()); 
    instance_info.ppEnabledLayerNames = layers.data();
    instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_info;
  }

  if (vkCreateInstance(&instance_info, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create vulkan instance");
  }
}

VkResult VulkanContext::CreateDebugUtilsMessengerEXT(
  VkInstance instance, 
  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
  const VkAllocationCallbacks* pAllocator, 
  VkDebugUtilsMessengerEXT* pDebugMessenger
) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void VulkanContext::DestroyDebugUtilsMessengerEXT(
  VkInstance instance, 
  VkDebugUtilsMessengerEXT debugMessenger, 
  const VkAllocationCallbacks* pAllocator
) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
      func(instance, debugMessenger, pAllocator);
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* pUserData
) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void VulkanContext::init_debugger() {
  if (!enabledValidationLayers) return;

  VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
  debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debugInfo.pfnUserCallback = debugCallback;

  if (CreateDebugUtilsMessengerEXT(instance, &debugInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create debug messenger");
  }
}

bool VulkanContext::check_validation_support() const {
  uint32_t propertyCount;
  vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);
  
  std::vector<VkLayerProperties> properties(propertyCount);
  vkEnumerateInstanceLayerProperties(&propertyCount, properties.data());

  for (auto required : layers) {
    bool contains = false;
    for (auto layer : properties) {
      if (std::strcmp(required, layer.layerName) == 0) {
        contains = true;
        break;
      }
    }
    if (!contains) {
      return false;
    }
  }

  return true;
}

void VulkanContext::init_surface(Window& window) {
  window.create_surface(instance, &surface);
}

void VulkanContext::pick_physical_device() {
  uint32_t device_count;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
  for (auto& device : devices) {
    if (is_device_suitable(device)) {
      physical_device = device;
      queue_index = find_queue_family(device);
      break;
    }
  }
}

bool VulkanContext::is_device_suitable(VkPhysicalDevice device) {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);
  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(device, &features);

  QueueIndices indices = find_queue_family(device);
  bool extension_supported = check_extension_support(device);

  bool condition = 
    properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && 
    features.fillModeNonSolid &&
    features.samplerAnisotropy &&
    features.vertexPipelineStoresAndAtomics &&
    indices.is_complete() && extension_supported;

  return condition;
}

QueueIndices VulkanContext::find_queue_family(VkPhysicalDevice device) {
  uint32_t queue_count;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queue_families.data());

  int index = 0;
  
  QueueIndices queue;

  for (const auto& family : queue_families) {
    
    if ((family.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (family.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
      VkBool32 present = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &present);
      if (present) {
        queue.index = index;        
      }
    }

    index++;
  }
  return queue;
}

bool VulkanContext::check_extension_support(VkPhysicalDevice device) const {
  uint32_t extension_count; 
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
  std::vector<VkExtensionProperties> extension_properties(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extension_properties.data());

  std::set<std::string> requiredExtensions(device_extensions.begin(), device_extensions.end()); 
  for (const auto& extension : extension_properties) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

void VulkanContext::init_device() {
  float priority = 1.0f;

  VkDeviceQueueCreateInfo queue_info{};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pQueuePriorities = &priority;
  queue_info.queueCount = 1;
  queue_info.queueFamilyIndex = queue_index.index.value();

  VkPhysicalDeviceFeatures features{};
  features.samplerAnisotropy = VK_TRUE;
  features.vertexPipelineStoresAndAtomics = VK_TRUE;

  VkDeviceCreateInfo device_info{};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.queueCreateInfoCount = 1;
  device_info.pEnabledFeatures = &features;
  device_info.pQueueCreateInfos = &queue_info;



  device_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
  device_info.ppEnabledExtensionNames = device_extensions.data();
  device_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
  device_info.ppEnabledLayerNames = layers.data();


  if (vkCreateDevice(physical_device, &device_info, nullptr, &device) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create device");
  }

  vkGetDeviceQueue(device, queue_index.index.value(), 0, &queue);
}


std::vector<const char*> VulkanContext::get_required_extensions() const {
  uint32_t extension_count;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extension_count); 
  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extension_count);
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  return extensions;
}

VkPhysicalDeviceLimits VulkanContext::find_device_limit() {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(physical_device, &properties);

  return properties.limits;

}
