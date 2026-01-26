#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#include <unordered_map>

class DescriptorLayoutCache {
  
  public:
		void init(VkDevice device);
		void cleanup();

		VkDescriptorSetLayout create_descriptor_layout(VkDescriptorSetLayoutCreateInfo* info);

		struct DescriptorLayoutInfo {
			std::vector<VkDescriptorSetLayoutBinding> bindings;

			bool operator==(const DescriptorLayoutInfo& other) const;

			size_t hash() const;
		};

  private:
    struct DescriptorLayoutHash		{
      std::size_t operator()(const DescriptorLayoutInfo& k) const {
        return k.hash();
      }
    };

		std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> cache;
		VkDevice device = VK_NULL_HANDLE;
};

