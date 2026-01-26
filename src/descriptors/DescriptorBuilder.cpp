
#include "DescriptorBuilder.hpp"

#include <iostream>

DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache* cache, DescriptorAllocator* allocator) {
	DescriptorBuilder builder;

	builder.cache = cache;
	builder.alloc = allocator;
	return builder;
}

DescriptorBuilder& DescriptorBuilder::DescriptorBuilder::bind_buffer(uint32_t binding, VkShaderStageFlags stageFlags,VkDescriptorType type, const VkDescriptorBufferInfo* buffer_info) {
		VkDescriptorSetLayoutBinding new_binding{};
		new_binding.descriptorCount = 1;
		new_binding.descriptorType = type;
		new_binding.pImmutableSamplers = nullptr;
		new_binding.stageFlags = stageFlags;
		new_binding.binding = binding;

		bindings.push_back(new_binding);

		VkWriteDescriptorSet new_write{};
		new_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		new_write.pNext = nullptr;

		new_write.descriptorCount = 1;
		new_write.descriptorType = type;
		new_write.pBufferInfo = buffer_info;
		new_write.dstBinding = binding;

		writes.push_back(new_write);
		return *this;
}

bool DescriptorBuilder::build(VkDescriptorSet& set, VkDescriptorSetLayout& layout){
	VkDescriptorSetLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.pNext = nullptr;
  
	layout_info.pBindings = bindings.data();
	layout_info.bindingCount = bindings.size();

	layout = cache->create_descriptor_layout(&layout_info);
	bool success = alloc->allocate(&set, layout);
	if (!success) { return false; };

	for (VkWriteDescriptorSet& w : writes) {
		w.dstSet = set;
	}

	vkUpdateDescriptorSets(alloc->device, writes.size(), writes.data(), 0, nullptr);
	return true;
}

void DescriptorBuilder::clear() {
  bindings.clear();
  writes.clear();
}
