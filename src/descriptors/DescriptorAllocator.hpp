
#pragma once

#include <vector>
#include <utility>
#include <vulkan/vulkan_core.h>

class DescriptorAllocator {
  
  public:
    struct PoolSizes {
      std::vector<std::pair<VkDescriptorType, float>> sizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1.0f},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1.0f}
      };
    };
  
    VkDescriptorPool create_pool(const PoolSizes& pool_sizes, int count, VkDescriptorPoolCreateFlags flags);
    void reset();
    bool allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);
    void init(VkDevice device);
    void cleanup();
  
    VkDevice device;
  private:
    VkDescriptorPool grabPool();

    VkDescriptorPool current_pool{VK_NULL_HANDLE};
    PoolSizes descriptor_sizes; 

		std::vector<VkDescriptorPool> used_pools;
		std::vector<VkDescriptorPool> free_pools;
};

