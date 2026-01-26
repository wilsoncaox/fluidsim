#pragma once

#include "../../descriptors/Descriptor.hpp"
#include "../Buffer.hpp"

class UniformBuffer : public Descriptor {
  
  public:
    UniformBuffer(VkDevice device, uint32_t binding, VkShaderStageFlags stage); 
    
    void bind(const Buffer* buffer); 
  private:

};
