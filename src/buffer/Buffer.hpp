#pragma once

#include "../command/CommandPool.hpp"

class Buffer {
  public:
    Buffer() = default;
    Buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~Buffer();

    void copyBuffer(Buffer& buffer, CommandPool& command_pool);

    VkDeviceSize size;
    VkBuffer buffer = VK_NULL_HANDLE;

  protected:
    uint32_t find_memory_type(uint32_t filter, VkMemoryPropertyFlags properties);

    VkDevice device;
    VkPhysicalDevice physical_device;
    VkDeviceMemory memory = VK_NULL_HANDLE; 
};
