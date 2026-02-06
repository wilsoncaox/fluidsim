#pragma once

#include "../../buffer/Buffer.hpp"
#include "../../pipeline/ComputePipeline.hpp"
#include "../../descriptors/DescriptorBuilder.hpp"

#include <memory>
#include <vector>


class Sort {

  public:
    // Have to ensure that data is binded before sorting is binded
    //
    Sort(VkDevice device, VkPhysicalDevice physical_device, uint32_t count, uint32_t size);
    void init(DescriptorBuilder& builder, VkDescriptorSetLayout data, uint32_t x, uint32_t y, uint32_t z); 

    void run(CommandPool& commandpool, VkCommandBuffer commandbuffer, VkBuffer data_buffer);
    void print_data(CommandPool& commandpool, VkPhysicalDevice physical_device);

  private:
    void init_temp(VkCommandBuffer commandbuffer, VkBuffer initial, CommandPool& commandpool);
    void extract_key(VkCommandBuffer commandbuffer, uint32_t data_index); 
    void reset_offset(VkCommandBuffer commandbuffer);
    void find_offset(VkCommandBuffer commandbuffer, uint32_t index, uint32_t data_index);
    void init_scans(VkCommandBuffer commandbuffer);
    void digit_scans(VkCommandBuffer commandbuffer, uint32_t index);
    size_t dispatch_scan(VkCommandBuffer commandbuffer, std::array<VkDescriptorSet, 2> sets, std::array<VkBuffer, 2> buffers);
    void partition_sort(VkCommandBuffer commandbuffer, size_t index, size_t read_index, size_t write_index, size_t zero_index, size_t one_index);
    void final_fill(VkCommandBuffer commandbuffer, VkBuffer src, VkBuffer dst);

    struct PushConstant {
      uint32_t particle_count;
      uint32_t index;
      uint32_t digit;
    };

    VkDevice device;
    VkPhysicalDevice physical_device;


    uint32_t groupCountX;
    uint32_t groupCountY;
    uint32_t groupCountZ;

    uint32_t data_count; 
    uint32_t data_size;


    VkDescriptorSet key_set;
    VkDescriptorSetLayout key_layout; 

    VkDescriptorSet offset_set;
    VkDescriptorSetLayout offset_layout;

    std::vector<VkDescriptorSet> scan_set;
    VkDescriptorSetLayout scan_layout;

    VkDescriptorSet prefix_set;
    VkDescriptorSetLayout prefix_layout;
  
    std::vector<VkDescriptorSet> data_temp_set;
    VkDescriptorSetLayout data_temp_layout;
    std::vector<Buffer> data_temp;

    std::unique_ptr<Buffer> keys;

    // One for read and write maybe jsut one for write
    // Maybe using 1 
    std::unique_ptr<Buffer> offset;
    
    // 2 * amount of digits we use
    // for 0 1 and we need 4 buffers for read and write
    std::vector<Buffer> scans;

    // 2 for only the prefix of 0 and 1;
    std::vector<Buffer> prefix;

    std::unique_ptr<ComputePipeline> key_pipeline;

    std::unique_ptr<ComputePipeline> offset_pipeline; 

    std::unique_ptr<ComputePipeline> copy_data_pipeline;

    std::unique_ptr<ComputePipeline> digits_pipeline;

    std::unique_ptr<ComputePipeline> scanning_pipeline; 

    std::unique_ptr<ComputePipeline> sorting_pipeline; 

};
