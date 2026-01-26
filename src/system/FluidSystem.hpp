
#pragma once

#include "../buffer/resource/StorageBuffer.hpp"
#include "System.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <vector>
#include <memory>

struct FluidData {
  glm::vec4 position;
  glm::vec4 velocity;
  glm::vec4 density;
  glm::vec4 predicted_position;
};

class FluidSystem : public System {
  
  public:
    FluidSystem(VkDevice device, VkPhysicalDevice physical_device, uint32_t buffer_count, uint32_t instance_count); 
    
    std::vector<Descriptor*> get_resource() const override;
    std::vector<VkBuffer> get_buffers() override;
    void init_data(CommandPool& commandpool, VkPhysicalDevice physical_device);

    void print_data(CommandPool& commandpool, VkPhysicalDevice physical_device, uint32_t frame);

    const float PI = 3.1415926538;


  private:

    uint32_t instance_count;

    std::vector<Buffer> buffers;
    std::vector<std::unique_ptr<StorageBuffer>> resources;
};
