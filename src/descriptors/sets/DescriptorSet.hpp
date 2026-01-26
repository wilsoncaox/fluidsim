#pragma once

#include <vulkan/vulkan_core.h>


// For binding to pipeline
class DescriptorSet {

  public:
    virtual void bind(VkCommandBuffer commandbuffer, uint32_t frame) = 0;
    virtual VkBuffer get_buffer(uint32_t index) = 0;
};
