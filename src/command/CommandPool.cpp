
#include "CommandPool.hpp"
#include <stdexcept>

CommandPool::CommandPool(VkDevice device, VkQueue queue, uint32_t queue_index) : device(device), queue(queue) {
  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = queue_index;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create command pool");
  }
 
}

void CommandPool::create_command_buffer(VkCommandBuffer* command_buffer, uint32_t count, VkCommandBufferLevel level) {
  VkCommandBufferAllocateInfo alloc_info{}; 
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool;
  alloc_info.commandBufferCount = count;
  alloc_info.level = level;
  
  if (vkAllocateCommandBuffers(device, &alloc_info, command_buffer) != VK_SUCCESS) {
    throw std::runtime_error("Unable to allocate command buffer");
  }
}

VkCommandBuffer CommandPool::start_single_command() {
  VkCommandBuffer command;
  create_command_buffer(&command, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; 
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(command, &begin_info);

  return command;
}

void CommandPool::end_single_command(VkCommandBuffer command) {
  vkEndCommandBuffer(command);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command;

  vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(queue);

  vkFreeCommandBuffers(device, command_pool, 1, &command);
}

