
#include "StorageBuffer.hpp"
#include <iostream>

StorageBuffer::StorageBuffer(
  VkDevice device, 
  uint32_t binding, 
  VkShaderStageFlags stage
) : Descriptor(device, binding, stage, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {}

void StorageBuffer::bind(const Buffer* buffer) {
  this->size = buffer->size;

  buffer_info.buffer = buffer->buffer;
  buffer_info.offset = 0;
  buffer_info.range = VK_WHOLE_SIZE;
}
