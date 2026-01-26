
#include "HostBuffer.hpp"

#include <cstring>

HostBuffer::HostBuffer(
  VkDevice device, 
  VkPhysicalDevice physical_device,
  VkDeviceSize size, 
  VkBufferUsageFlags usage
) : Buffer(device, physical_device, size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {}

void HostBuffer::fillData(void* values, uint32_t dataSize) {
  void* data;
  vkMapMemory(device, memory, 0, dataSize, 0, &data);
  memcpy(data, values, (size_t) dataSize);
  vkUnmapMemory(device, memory);
}

void HostBuffer::getData(void* values) {
  void* data;
  vkMapMemory(device, memory, 0, size, 0, &data);

  memcpy(values, data, (size_t) size);
  vkUnmapMemory(device, memory);
}

