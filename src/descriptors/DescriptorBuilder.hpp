
#pragma once

#include "DescriptorLayoutCache.hpp"
#include "DescriptorAllocator.hpp"


class DescriptorBuilder {

  public:
    static DescriptorBuilder begin(DescriptorLayoutCache* cache, DescriptorAllocator* allocator );

    DescriptorBuilder& bind_buffer(uint32_t binding, VkShaderStageFlags stageFlags,VkDescriptorType type, const VkDescriptorBufferInfo* buffer_info);

    bool build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
    bool build(VkDescriptorSet& set); 

    void clear();

  private:
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    DescriptorLayoutCache* cache;
    DescriptorAllocator* alloc;
};

