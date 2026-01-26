
#pragma once

#include "DescriptorSet.hpp"

#include <vector>

class CameraDescriptorSet : public DescriptorSet {

  public:
    CameraDescriptorSet(VkPipelineLayout layout, std::vector<VkDescriptorSet> descriptor_sets, uint32_t offset); 

    void bind(VkCommandBuffer commandbuffer, uint32_t frame) override;
    VkBuffer get_buffer(uint32_t index) override { return VK_NULL_HANDLE; };

  
  private:
    std::vector<VkDescriptorSet> descriptor_sets;
    VkPipelineLayout layout;
    uint32_t offset;
};
