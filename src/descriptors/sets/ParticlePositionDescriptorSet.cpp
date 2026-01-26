


#include "ParticlePositionDescriptorSet.hpp"

ParticlePositionDescriptorSet::ParticlePositionDescriptorSet(
  VkPipelineLayout layout,
  VkPipelineBindPoint bind_point,
  std::vector<VkBuffer> buffers,
  std::vector<VkDescriptorSet> descriptor_sets,
  uint32_t offset,
  uint32_t set_binding
) : layout(layout), bind_point(bind_point), buffers(buffers), descriptor_sets(descriptor_sets), offset(offset), set_binding(set_binding) {}


void ParticlePositionDescriptorSet::bind(VkCommandBuffer commandbuffer, uint32_t frame) {
  uint32_t new_frame = (frame*3 + offset) % descriptor_sets.size();
  vkCmdBindDescriptorSets(commandbuffer, bind_point, layout, set_binding, 1, &descriptor_sets[new_frame], 0, nullptr);  
}

VkBuffer ParticlePositionDescriptorSet::get_buffer(uint32_t current_frame) {
  uint32_t new_frame = (current_frame*3 + offset) % descriptor_sets.size();
  return buffers[new_frame];
}

