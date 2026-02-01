#pragma once


#include <vulkan/vulkan_core.h>

class Descriptor {
  
  public:
    Descriptor(
      VkDevice device, 
      uint32_t binding, 
      VkShaderStageFlags stage, 
      VkDescriptorType type
    ) : device(device), binding(binding), stage(stage), type(type) {};

    const VkDescriptorBufferInfo& get_buffer_info() const { return buffer_info; };
    const VkDeviceSize get_size() const { return size; };

    uint32_t binding; 
    VkShaderStageFlags stage;
    VkDescriptorType type;
    
  protected:
    VkDevice device;

    VkDescriptorBufferInfo buffer_info;
    VkDeviceSize size;
};
