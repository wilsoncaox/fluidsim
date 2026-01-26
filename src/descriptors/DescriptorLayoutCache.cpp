
#include "DescriptorLayoutCache.hpp"

#include <algorithm>
#include <iostream>

void DescriptorLayoutCache::init(VkDevice device) {
	this->device = device;
}
void DescriptorLayoutCache::cleanup() {
	for (auto pair : cache){
		vkDestroyDescriptorSetLayout(device, pair.second, nullptr);
	}
}

VkDescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(VkDescriptorSetLayoutCreateInfo* info) {
	DescriptorLayoutInfo layout_info;
	layout_info.bindings.reserve(info->bindingCount);
	bool isSorted = true;
	int lastBinding = -1;

	for (int i = 0; i < info->bindingCount; i++) {
		layout_info.bindings.push_back(info->pBindings[i]);

		if (info->pBindings[i].binding > lastBinding) {
			lastBinding = info->pBindings[i].binding;
		} else {
			isSorted = false;
		}
	}

	if (!isSorted){
		std::sort(layout_info.bindings.begin(), layout_info.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b ){
				return a.binding < b.binding;
		});
	}

  auto it = cache.find(layout_info);
  if (it != cache.end()){
    return (*it).second;
  } else {
    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(device, info, nullptr, &layout);

    cache[layout_info] = layout;
    return layout;
  }
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const {
	if (other.bindings.size() != bindings.size()) {
		return false;
	} else {
		for (size_t i = 0; i < bindings.size(); i++) {
			if (other.bindings[i].binding != bindings[i].binding) {
				return false;
			}
			if (other.bindings[i].descriptorType != bindings[i].descriptorType) {
				return false;
			}
			if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) {
				return false;
			}
			if (other.bindings[i].stageFlags != bindings[i].stageFlags) {
				return false;
			}
		}
		return true;
	}
}

size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const {
  using std::size_t;
  using std::hash;
  
  size_t result = hash<size_t>()(bindings.size());
  for (const VkDescriptorSetLayoutBinding& b : bindings) {
    size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

    result ^= hash<size_t>()(binding_hash);
  }

  return result;
}



