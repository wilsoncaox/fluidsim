
#include "DescriptorAllocator.hpp"
#include "DescriptorBuilder.hpp"
#include "DescriptorLayoutCache.hpp"

class DescriptorHandler {

  public:
    void init(VkDevice device);

void bind_descriptor(uint32_t binding, VkShaderStageFlags stage,VkDescriptorType type, const VkDescriptorBufferInfo* buffer_info);
    void build_descriptor(VkDescriptorSet& descriptor_set, VkDescriptorSetLayout& descriptor_layout);
    void clear_descriptor();

    DescriptorBuilder descriptor_builder;
  private:
    VkDevice device = VK_NULL_HANDLE;

    DescriptorAllocator allocator;
    DescriptorLayoutCache layout_cache;

};
