#pragma once

#include <vulkan/vulkan_core.h>

class Pipeline {

  public:
    Pipeline(VkDevice device) : device(device) {};

    virtual void bind_pipeline(VkCommandBuffer command_buffer) = 0;
    
    VkPipelineLayout get_pipeline_layout() { return layout; };
    void bind_push_constants(VkCommandBuffer commandbuffer, VkShaderStageFlags stage, uint32_t size, const void *values) { 
      vkCmdPushConstants(commandbuffer, layout, stage, 0, size, values);
    }
    void bind_descriptor_sets(VkCommandBuffer commandbuffer, VkPipelineBindPoint bindpoint, uint32_t set_binding, uint32_t descriptor_count, const VkDescriptorSet* descriptor_sets) {
      vkCmdBindDescriptorSets(commandbuffer, bindpoint, layout, set_binding, descriptor_count, descriptor_sets, 0, nullptr);
    }

  protected:
    VkDevice device;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
};
