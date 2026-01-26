
#include "DescriptorAllocator.hpp"


#include "DescriptorAllocator.hpp"

void DescriptorAllocator::init(VkDevice device) {
  this->device = device;
}

void DescriptorAllocator::cleanup() {
  for (auto p : free_pools)
  {
    vkDestroyDescriptorPool(device, p, nullptr);
  }
  for (auto p : used_pools)
  {
    vkDestroyDescriptorPool(device, p, nullptr);
  }
}


VkDescriptorPool DescriptorAllocator::create_pool(const DescriptorAllocator::PoolSizes& pool_sizes, int count, VkDescriptorPoolCreateFlags flags) {
  std::vector<VkDescriptorPoolSize> sizes;
  sizes.reserve(pool_sizes.sizes.size());

  for (auto sz : pool_sizes.sizes) {
    sizes.push_back({sz.first, uint32_t(sz.second * count)});
  }

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = flags;
  pool_info.maxSets = count;
  pool_info.poolSizeCount = (uint32_t)sizes.size();
  pool_info.pPoolSizes = sizes.data();

  VkDescriptorPool descriptor_pool;
  vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool);

  return descriptor_pool;
}

VkDescriptorPool DescriptorAllocator::grabPool() {
  if (free_pools.size() > 0) {
    VkDescriptorPool pool = free_pools.back();
    free_pools.pop_back();
    return pool;
  } else {
    return create_pool(descriptor_sizes, 100, 0);
  }
}

bool DescriptorAllocator::allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout) {
  if (current_pool == VK_NULL_HANDLE) {
    current_pool = grabPool();
    used_pools.push_back(current_pool);
  }

  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;

  alloc_info.pSetLayouts = &layout;
  alloc_info.descriptorPool = current_pool;
  alloc_info.descriptorSetCount = 1;

  VkResult result = vkAllocateDescriptorSets(device, &alloc_info, set);
  bool need_reallocate = false;

  switch (result) {
    case VK_SUCCESS:
      return true;
    case VK_ERROR_FRAGMENTED_POOL:
    case VK_ERROR_OUT_OF_POOL_MEMORY:
      need_reallocate = true;
      break;
    default:
      return false;
  }

  if (need_reallocate) {
    current_pool = grabPool();
    used_pools.push_back(current_pool);

    result = vkAllocateDescriptorSets(device, &alloc_info, set);

    if (result == VK_SUCCESS) {
      return true;
    }
  }

  return false;
}

void DescriptorAllocator::reset(){
  for (auto p : used_pools){
    vkResetDescriptorPool(device, p, 0);
    free_pools.push_back(p);
  }

  used_pools.clear();

  current_pool = VK_NULL_HANDLE;
}



