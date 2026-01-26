#pragma once

#include <vulkan/vulkan_core.h>

class CommandPool {

  public:
    CommandPool(VkDevice device, VkQueue queue, uint32_t queue_index);

    void create_command_buffer(VkCommandBuffer* command_buffer, uint32_t count, VkCommandBufferLevel level);

    VkCommandBuffer start_single_command();
    void end_single_command(VkCommandBuffer command_buffer);

    void free_command_buffer(VkCommandBuffer* command_buffer);

  private:
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;

    VkCommandPool command_pool = VK_NULL_HANDLE;
};
