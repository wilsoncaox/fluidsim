
#include "DescriptorHandler.hpp"
#include <iostream>
#include <stdexcept>

void DescriptorHandler::init(VkDevice device) {
  allocator.init(device);
  layout_cache.init(device);
  descriptor_builder = DescriptorBuilder::begin(&layout_cache, &allocator);
}

void DescriptorHandler::bind_descriptor(uint32_t binding, VkShaderStageFlags stage,VkDescriptorType type, const VkDescriptorBufferInfo* buffer_info) {
  descriptor_builder.bind_buffer(binding, stage, type, buffer_info);
}

void DescriptorHandler::build_descriptor(VkDescriptorSet& set, VkDescriptorSetLayout& layout) {
  if (!descriptor_builder.build(set, layout)) {
    throw std::runtime_error("Unable to build descriptor");
  }
}

void DescriptorHandler::clear_descriptor() {
  descriptor_builder.clear();
}
