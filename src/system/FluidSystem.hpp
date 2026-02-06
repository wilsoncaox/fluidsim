
#pragma once

#include "../command/CommandPool.hpp"
#include "../buffer/Buffer.hpp"
#include "../buffer/HostBuffer.hpp"
#include "../context/Window.hpp"
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

    void update_boundary(Window& window);

    void run(CommandPool& commandpool, VkCommandBuffer commandbuffer);
    void bind_particle(VkCommandBuffer commandbuffer, Pipeline& pipeline, VkPipelineBindPoint bind_point);

    const float PI = 3.1415926538;

    VkDescriptorSetLayout particle_layout_graphics;
  private:
    void calculate_predicted_position(VkCommandBuffer commandbuffer);
    void update_spatial_lookup(VkCommandBuffer commandbuffer, CommandPool& commandpool);
    void calculate_density(VkCommandBuffer commandbuffer);
    void move_particles(VkCommandBuffer commandbuffer);
    void init_boundary();

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

    VkDescriptorSet boundary_set;
    VkDescriptorSetLayout boundary_layout;
    std::unique_ptr<HostBuffer> boundary_buffer;

    std::unique_ptr<ComputePipeline> position_pipline;
    std::unique_ptr<ComputePipeline> spatial_pipeline;
    std::unique_ptr<ComputePipeline> density_pipeline;
    std::unique_ptr<ComputePipeline> move_pipeline;

    uint32_t read_index = 0;
    uint32_t write_index = 1;

    const int table_cells = 17658;

    float front ;
    float back;
    float bottom;
    float top;
    float right;
    float left;

};


