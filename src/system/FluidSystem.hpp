
#pragma once

#include "../command/CommandPool.hpp"
#include "../buffer/Buffer.hpp"
#include "subsystem/Sort.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <vector>
#include <memory>

struct FluidData {
  // Position x, y, z first 3 slots and final slot is key
  glm::vec4 position;
  glm::vec4 velocity;
  glm::vec4 predicted_position;
};

class FluidSystem {

  public:
    FluidSystem(VkDevice device, VkPhysicalDevice physical_device, DescriptorBuilder& builder, uint32_t instance_count); 
    
    void init_data(CommandPool& commandpool, VkPhysicalDevice physical_device);

    void print_data(CommandPool& commandpool, VkPhysicalDevice physical_device);
    void print_density(CommandPool& commandpool, VkPhysicalDevice pysical_device);


    void run(CommandPool& commandpool, VkCommandBuffer commandbuffer);
    void bind_particle(VkCommandBuffer commandbuffer, Pipeline& pipeline, VkPipelineBindPoint bind_point);

    const float PI = 3.1415926538;

    VkDescriptorSetLayout particle_layout_graphics;
  private:
    void calculate_predicted_position(VkCommandBuffer commandbuffer);
    void update_spatial_lookup(VkCommandBuffer commandbuffer, CommandPool& commandpool);
    void calculate_density(VkCommandBuffer commandbuffer);
    void move_particles(VkCommandBuffer commandbuffer);

    VkDevice device;
    VkPhysicalDevice physical_device;

    uint32_t instance_count;

    std::vector<VkDescriptorSet> particle_set;
    std::vector<VkDescriptorSet> particle_set_graphics;
    std::vector<Buffer> particle_buffers;
    VkDescriptorSetLayout particle_layout;

    VkDescriptorSetLayout position_layout;
    VkDescriptorSet position_set;
    std::unique_ptr<Buffer> position_buffer;

    std::unique_ptr<CommandPool> commandpool;
    std::unique_ptr<Sort> sort;

    VkDescriptorSet spatial_lookup_set;
    VkDescriptorSetLayout spatial_lookup_layout;
    std::unique_ptr<Buffer> spatial_lookup_buffer;

    VkDescriptorSet density_set;
    VkDescriptorSetLayout density_layout;
    std::unique_ptr<Buffer> density_buffer;

    std::unique_ptr<ComputePipeline> position_pipline;
    std::unique_ptr<ComputePipeline> spatial_pipeline;
    std::unique_ptr<ComputePipeline> density_pipeline;
    std::unique_ptr<ComputePipeline> move_pipeline;

    uint32_t read_index = 0;
    uint32_t write_index = 1;

    const int hashK1 = 15823;
    const int hashK2 = 9737333;
    const int hashK3 = 9737357;
    const int table_cells = 17658;

    const float smoothing_radius = 0.2;

    int key_from_hash(int hash) {
      int count = int(instance_count);
      int k = hash % count;
      if (k < 0) k += count;
      return k;
    }

    int grid_from_pos(float value) {
      return int(std::floor(value / smoothing_radius));
    }

    int get_key(glm::vec4 position) {
      int grid_x = grid_from_pos(position.x);
      int grid_y = grid_from_pos(position.y);
      int grid_z = grid_from_pos(position.z);

      int hash = grid_x * hashK1 + grid_y * hashK2 + grid_z*hashK3;
      int key = std::abs(key_from_hash(hash));
      return key;
    }

};
