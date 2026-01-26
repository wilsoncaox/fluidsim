#pragma once

#include "../descriptors/Descriptor.hpp"
#include "../command/CommandPool.hpp"

#include <vector>

class System {

  public:
    System(VkDevice device) : device(device) {};  

    virtual std::vector<Descriptor*> get_resource() const = 0;
    virtual std::vector<VkBuffer> get_buffers() = 0;

  protected:
    VkDevice device;
};
