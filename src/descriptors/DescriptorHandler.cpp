
#include "DescriptorHandler.hpp"
#include <iostream>
#include <stdexcept>

void DescriptorHandler::init(VkDevice device) {
  allocator.init(device);
  layout_cache.init(device);
  descriptor_builder = DescriptorBuilder::begin(&layout_cache, &allocator);
}


void DescriptorHandler::bind_descriptor(Descriptor* descriptor) {
  descriptor_builder.bind_buffer(descriptor->binding, descriptor->stage, descriptor->type, &descriptor->get_buffer_info());
}

void DescriptorHandler::build_descriptor(VkDescriptorSet& set, VkDescriptorSetLayout& layout) {
  if (!descriptor_builder.build(set, layout)) {
    throw std::runtime_error("Unable to build descriptor");
  }
}

void DescriptorHandler::clear_descriptor() {
  descriptor_builder.clear();
}

void DescriptorHandler::bind_compute(VkCommandBuffer commandbuffer, uint32_t frame) {
  for (auto& set : compute_sets) {
    set->bind(commandbuffer, frame);
  }
}


void DescriptorHandler::bind_graphics(VkCommandBuffer commandbuffer, uint32_t frame) {
  for (auto& set : graphics_sets) {
    set->bind(commandbuffer, frame);
  }
}

void DescriptorHandler::add_descriptor_set_compute(std::unique_ptr<DescriptorSet> descriptor_set) {
  compute_sets.push_back(std::move(descriptor_set));
}


void DescriptorHandler::add_descriptor_set_graphics(std::unique_ptr<DescriptorSet> descriptor_set) {
  graphics_sets.push_back(std::move(descriptor_set));
}
