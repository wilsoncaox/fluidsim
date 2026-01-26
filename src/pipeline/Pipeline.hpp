#pragma once

#include <vulkan/vulkan_core.h>

class Pipeline {

  public:
    Pipeline(VkDevice device) : device(device) {};

    virtual void bind_pipeline(VkCommandBuffer command_buffer) = 0;
    VkPipelineLayout get_pipeline_layout() { return layout; };

  protected:
    VkDevice device;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
};
