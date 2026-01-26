
#include "Buffer.hpp"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

Buffer::Buffer(
  VkDevice device, 
  VkPhysicalDevice physical_device,
  VkDeviceSize size, 
  VkBufferUsageFlags usage, 
  VkMemoryPropertyFlags properties
) : device(device), physical_device(physical_device), size(size) {

  VkBufferCreateInfo buffer_info{};  
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buffer_info.usage = usage;

  if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("UNable to create buffer");
  }

  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(device, buffer, &mem_requirements);

  VkMemoryAllocateInfo alloc_info{}; 
  alloc_info.sType= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }
  
  vkBindBufferMemory(device, buffer, memory, 0);
}

Buffer::~Buffer() {
  if (memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, memory, nullptr);
  }

  if (buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, buffer, nullptr);
  }
}

uint32_t Buffer::find_memory_type(uint32_t filter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i) {
    if ((filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}


void Buffer::copyBuffer(Buffer& src, CommandPool& command_pool) {
  if (size != src.size) {
    throw std::runtime_error("Copying buffer needs to have same size");
  }
  
  VkCommandBuffer command_buffer = command_pool.start_single_command();

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(command_buffer, src.buffer, buffer, 1, &copyRegion);

  command_pool.end_single_command(command_buffer); 
}
