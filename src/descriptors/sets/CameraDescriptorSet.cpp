
#include "CameraDescriptorSet.hpp"
#include <vulkan/vulkan_core.h>

CameraDescriptorSet::CameraDescriptorSet(
  VkPipelineLayout layout,
  std::vector<VkDescriptorSet> descriptor_sets,
  uint32_t offset
) : layout(layout), descriptor_sets(descriptor_sets), offset(offset) {}


void CameraDescriptorSet::bind(VkCommandBuffer commandbuffer, uint32_t frame) {
  uint32_t new_frame = (frame + offset) % descriptor_sets.size();
  vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptor_sets[new_frame], 0, nullptr);  
}

