#pragma once

#include "Buffer.hpp"

class HostBuffer : public Buffer {
  
  public:
    HostBuffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage);

    void fillData(void* values, uint32_t dataSize);
    void getData(void* values);

};
