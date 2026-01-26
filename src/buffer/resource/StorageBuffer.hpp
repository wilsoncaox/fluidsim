#pragma once


#include "../../descriptors/Descriptor.hpp"
#include "../Buffer.hpp"


class StorageBuffer : public Descriptor {
 
  public:
    StorageBuffer(VkDevice device, uint32_t binding, VkShaderStageFlags stage); 
    
    void bind(const Buffer* buffer); 

};
