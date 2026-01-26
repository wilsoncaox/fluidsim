
#include "DescriptorAllocator.hpp"
#include "DescriptorBuilder.hpp"
#include "DescriptorLayoutCache.hpp"
#include "Descriptor.hpp"
#include "sets/DescriptorSet.hpp"

#include <vector>
#include <memory>

class DescriptorHandler {

  public:
    void init(VkDevice device);

    void bind_descriptor(Descriptor* descriptor);
    void build_descriptor(VkDescriptorSet& descriptor_set, VkDescriptorSetLayout& descriptor_layout);
    void clear_descriptor();

    void add_descriptor_set_compute(std::unique_ptr<DescriptorSet> descriptor_set);
    void add_descriptor_set_graphics(std::unique_ptr<DescriptorSet> descriptor_set);

    void bind_compute(VkCommandBuffer commandbuffer, uint32_t frame);
    void bind_graphics(VkCommandBuffer commandbuffer, uint32_t frame);



    std::unique_ptr<DescriptorSet> compute_particle_scan_write;
    std::unique_ptr<DescriptorSet> compute_particle_scan_read;
    std::unique_ptr<DescriptorSet> compute_particle_histogram;
    std::unique_ptr<DescriptorSet> compute_particle_position_write;
    std::unique_ptr<DescriptorSet> compute_particle_position_read;
    std::unique_ptr<DescriptorSet> compute_particle_density_write;
    std::unique_ptr<DescriptorSet> compute_particle_density_read;
    std::unique_ptr<DescriptorSet> compute_particle_read;
    std::unique_ptr<DescriptorSet> compute_particle_write;
    std::unique_ptr<DescriptorSet> graphics_particle_set;
    std::unique_ptr<DescriptorSet> graphics_camera_set;

  private:
    VkDevice device = VK_NULL_HANDLE;
 
    std::vector<std::unique_ptr<DescriptorSet>> compute_sets;
    std::vector<std::unique_ptr<DescriptorSet>> graphics_sets;

    DescriptorAllocator allocator;
    DescriptorLayoutCache layout_cache;
    DescriptorBuilder descriptor_builder;

};
