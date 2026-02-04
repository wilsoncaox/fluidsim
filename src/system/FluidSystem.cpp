
#include "FluidSystem.hpp"
#include "../buffer/HostBuffer.hpp"

#include <random>
#include <iostream>
#include <cmath>
#include <array>
#include <limits>
#include <vulkan/vulkan_core.h>

FluidSystem::FluidSystem(
  VkDevice device, 
  VkPhysicalDevice physical_device, 
  DescriptorBuilder& builder,
  uint32_t instance_count
) : device(device), physical_device(physical_device), instance_count(instance_count) {

  particle_buffers.reserve(2);  

  for (size_t i = 0; i < 2; i++) {
    particle_buffers.emplace_back(
        device, 
        physical_device, 
        sizeof(FluidData)*instance_count, 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
  }

  position_buffer = std::make_unique<Buffer>(
    device, 
    physical_device, 
    sizeof(FluidData)*instance_count, 
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );

  spatial_lookup_buffer = std::make_unique<Buffer>(
    device, 
    physical_device, 
    sizeof(uint32_t)*table_cells,
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );

  density_buffer = std::make_unique<Buffer>(
    device,
    physical_device,
    sizeof(float)*instance_count,
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );

  // For compute
  particle_set.resize(2);
  for (size_t i = 0; i < 2; i++) {
    builder.bind_buffer(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, particle_buffers[i].get_info()); 
    builder.build(particle_set[i], particle_layout);
    builder.clear();
  }

  // For graphics
  particle_set_graphics.resize(2);
  for (size_t i = 0; i < 2; i++) {
    builder.bind_buffer(0, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, particle_buffers[i].get_info()); 
    builder.build(particle_set_graphics[i], particle_layout_graphics);
    builder.clear();
  }


  builder.bind_buffer(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, position_buffer->get_info());
  builder.build(position_set, position_layout);
  builder.clear();

  builder.bind_buffer(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, spatial_lookup_buffer->get_info());
  builder.build(spatial_lookup_set, spatial_lookup_layout);
  builder.clear();

  builder.bind_buffer(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, density_buffer->get_info());
  builder.build(density_set, density_layout);
  builder.clear();

  // Key descriptors 
  sort = std::make_unique<Sort>(device, physical_device, instance_count, sizeof(FluidData));
  sort->init(builder, particle_layout, (instance_count / 256) + 1, 1, 1);

  VkPushConstantRange particle_constant{};
  particle_constant.size = sizeof(uint32_t);
  particle_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  spatial_pipeline = std::make_unique<ComputePipeline>(device, "shaders/vertex.spatial.comp.spv"); 
  spatial_pipeline->create({particle_layout, spatial_lookup_layout}, {particle_constant});

  position_pipline = std::make_unique<ComputePipeline>(device, "shaders/vertex.position.comp.spv");
  position_pipline->create({position_layout, particle_layout}, {particle_constant});

  density_pipeline = std::make_unique<ComputePipeline>(device, "shaders/vertex.density.comp.spv");   
  density_pipeline->create({particle_layout, density_layout, spatial_lookup_layout}, {particle_constant});

  move_pipeline = std::make_unique<ComputePipeline>(device, "shaders/vertex.move.comp.spv");
  move_pipeline->create({particle_layout, particle_layout, density_layout, spatial_lookup_layout}, {particle_constant});

};

void FluidSystem::calculate_predicted_position(VkCommandBuffer commandbuffer) {
  VkBufferCopy region{};
  region.size = sizeof(FluidData) * instance_count;
  vkCmdCopyBuffer(commandbuffer, particle_buffers[read_index].buffer, position_buffer->buffer, 1, &region);

  VkBufferMemoryBarrier position_copy_barrier{};
  position_copy_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  position_copy_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  position_copy_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  position_copy_barrier.buffer = position_buffer->buffer;
  position_copy_barrier.offset = 0;
  position_copy_barrier.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(
    commandbuffer,
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    0,
    0, nullptr,
    1, &position_copy_barrier,
    0, nullptr
  );


  std::array<VkDescriptorSet, 2> sets = {position_set, particle_set[read_index]}; 
  position_pipline->bind_descriptor_sets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 0, 2, sets.data()); 
  position_pipline->bind_push_constants(commandbuffer, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t), &instance_count);
  position_pipline->bind_pipeline(commandbuffer);
  vkCmdDispatch(commandbuffer, (instance_count / 256) + 1, 1, 1);

  VkBufferMemoryBarrier position_barrier{};
  position_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  position_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  position_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  position_barrier.buffer = particle_buffers[read_index].buffer;
  position_barrier.offset = 0;
  position_barrier.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(
    commandbuffer,
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    0,
    0, nullptr,
    1, &position_barrier,
    0, nullptr
  );
}

void FluidSystem::update_spatial_lookup(VkCommandBuffer commandbuffer, CommandPool& commandpool) {
  sort->run(commandpool, commandbuffer, particle_buffers[read_index].buffer);
  
  vkCmdFillBuffer(commandbuffer, spatial_lookup_buffer->buffer, 0, spatial_lookup_buffer->size, std::numeric_limits<uint32_t>::max());

  VkBufferMemoryBarrier spatial_default_barrier{};
  spatial_default_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  spatial_default_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  spatial_default_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  spatial_default_barrier.buffer = spatial_lookup_buffer->buffer;
  spatial_default_barrier.offset = 0;
  spatial_default_barrier.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(
    commandbuffer,
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    0,
    0, nullptr,
    1, &spatial_default_barrier,
    0, nullptr
  );


  std::array<VkDescriptorSet, 2> sets = {particle_set[read_index], spatial_lookup_set};
  spatial_pipeline->bind_descriptor_sets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 0, static_cast<uint32_t>(sets.size()), sets.data());
  spatial_pipeline->bind_push_constants(commandbuffer, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t), &instance_count);
  spatial_pipeline->bind_pipeline(commandbuffer);
  vkCmdDispatch(commandbuffer, (instance_count / 256) + 1, 1, 1);

  VkBufferMemoryBarrier barrier{};
  VkBufferMemoryBarrier position_barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  barrier.buffer = spatial_lookup_buffer->buffer;
  barrier.offset = 0;
  barrier.size = VK_WHOLE_SIZE;


  vkCmdPipelineBarrier(
    commandbuffer,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    0,
    0, nullptr,
    1, &barrier,
    0, nullptr
  );

}

void FluidSystem::calculate_density(VkCommandBuffer commandbuffer){ 
  std::array<VkDescriptorSet, 3> sets = { particle_set[read_index], density_set, spatial_lookup_set };
  density_pipeline->bind_descriptor_sets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 0, static_cast<uint32_t>(sets.size()), sets.data());
  density_pipeline->bind_push_constants(commandbuffer, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(float), &instance_count);
  density_pipeline->bind_pipeline(commandbuffer);
  vkCmdDispatch(commandbuffer, (instance_count / 256) + 1, 1, 1);

  VkBufferMemoryBarrier density_barrier{};
  density_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  density_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  density_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  density_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  density_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  density_barrier.buffer = density_buffer->buffer;
  density_barrier.offset = 0;
  density_barrier.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(
    commandbuffer, 
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
    0, 
    0, nullptr, 
    1, &density_barrier, 
    0, nullptr
  );
}

void FluidSystem::move_particles(VkCommandBuffer commandbuffer) {

  std::array<VkDescriptorSet, 4> sets = {particle_set[read_index], particle_set[write_index], density_set, spatial_lookup_set};

  move_pipeline->bind_descriptor_sets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 0, static_cast<uint32_t>(sets.size()), sets.data());
  move_pipeline->bind_push_constants(commandbuffer, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t), &instance_count);
  move_pipeline->bind_pipeline(commandbuffer);  
  vkCmdDispatch(commandbuffer, (instance_count / 256) + 1, 1, 1);

  VkBufferMemoryBarrier move_barrier{};
  move_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  move_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  move_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  move_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  move_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  move_barrier.buffer = particle_buffers[write_index].buffer;
  move_barrier.offset = 0;
  move_barrier.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(
    commandbuffer, 
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
    0, 
    0, nullptr, 
    1, &move_barrier, 
    0, nullptr
  );
}

void FluidSystem::run(CommandPool& commandpool, VkCommandBuffer commandbuffer) {
  calculate_predicted_position(commandbuffer);
  update_spatial_lookup(commandbuffer, commandpool);
  calculate_density(commandbuffer); 
  move_particles(commandbuffer);

  read_index = (read_index + 1) % 2;
  write_index = (write_index + 1) % 2;
}


void FluidSystem::bind_particle(VkCommandBuffer commandbuffer, Pipeline& pipeline, VkPipelineBindPoint bind_point) {
  pipeline.bind_descriptor_sets(commandbuffer, bind_point, 0, 1, &particle_set_graphics[read_index]);
}

void FluidSystem::init_data(CommandPool& commandpool, VkPhysicalDevice physical_device) {
  VkDeviceSize size = sizeof(FluidData) * instance_count; 

  HostBuffer staging(
    device, 
    physical_device, 
    size, 
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT
  );

  std::vector<FluidData> values;
  values.reserve(instance_count);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(-5.0f, 5.0f);
  for (int i = 0; i < instance_count; ++i) {
    float x = dist(gen);
    float y = dist(gen);
    float z = dist(gen);


    FluidData data{};
    data.position = {x, y ,z, 0};
    values.push_back(data);
  }

  // FluidData data1{};
  // data1.position = {0.2, 0.2, 0.0, 0}; // (0, 0, 0);
  //
  // FluidData data2{};
  // data2.position = {0.7, 0.2, 0.0, 0}; // (1, 0, 0)
  //
  // FluidData data3{};
  // data3.position = {0.8, 0.6, 0.0, 0}; // (1, 1, 0)
  //
  // FluidData data4{};
  // data4.position = {-0.3, 0.1, 0.0, 0}; // (-1, 0, 0)
  //
  // FluidData data5{};
  // data5.position = {-0.2, -0.1, 0.0, 0}; // (-1, -1, 0)
  //
  // FluidData data6{};
  // data6.position = {0.8, -0.2, 0.0, 0}; // (1, -1, 0)
  //
  // FluidData data7{};
  // data7.position = {-0.2, 0.8, 0.0, 0}; // (-1, 1, 0)
  //
  // FluidData data8{};
  // data8.position = {0.3, 0.8, 0.0, 0}; // (0, 1, 0)
  //
  // FluidData data9{};
  // data9.position = {0.2, -0.2, 0.0, 0}; // (0, -1, 0)
  //
  // FluidData data10{};
  // data10.position = {1.2, 0.2, 0.0, 0}; // (2, 0 ,0)
  //
  // FluidData data11{};
  // data11.position = {1.24, 0.42, 0.0, 0}; // (2, 0 ,0)
  //
  // FluidData data12{};
  // data12.position = {1.33, 0.12, 0.0, 0}; // (2, 0 ,0)
  //
  // FluidData data13{};
  // data13.position = {1.33, 0.01, 0.0, 0}; // (2, 0 ,0)
  //
  // FluidData data14{};
  // data14.position = {1.18, 0.01, 0.0, 0}; // (2, 0 ,0)
  //
  // values.push_back(data1);
  // values.push_back(data2);
  // values.push_back(data3);
  // values.push_back(data4);
  // values.push_back(data5);
  // values.push_back(data6);
  // values.push_back(data7);
  // values.push_back(data8);
  // values.push_back(data9);
  // values.push_back(data10);
  // values.push_back(data11);
  // values.push_back(data12);
  // values.push_back(data13);
  // values.push_back(data14);
  //
  //
  // for (auto i : values) {
  //   int hash = grid_from_pos(i.position.x)*15823 + grid_from_pos(i.position.y)*9737333 + grid_from_pos(i.position.z)*9737357;  
  //   std::cout << i.position.x << " " << i.position.y << " " << i.position.z << " key: " << (hash % instance_count) << '\n';
  // }
  //
  staging.fillData(values.data(), size);

  for (auto& buffer : particle_buffers) {
    buffer.copyBuffer(staging, commandpool);
  }
}


void FluidSystem::print_data(CommandPool& commandpool, VkPhysicalDevice physical_device) {
  vkDeviceWaitIdle(device);
  VkDeviceSize size = sizeof(FluidData) * instance_count; 
  HostBuffer staging(
    device, 
    physical_device, 
    size, 
    VK_BUFFER_USAGE_TRANSFER_DST_BIT
  );

  staging.copyBuffer(particle_buffers[write_index], commandpool);

  std::vector<FluidData> reading(instance_count);
  staging.getData(reading.data());

  // staging.copyBuffer(particle_buffers[write_index], commandpool);
  // std::vector<FluidData> writing(instance_count);
  // staging.getData(writing.data());

  // Key
  for (size_t i = 0; i < instance_count; i++) {
    std::cout << reading[i].predicted_position.x << " " << reading[i].predicted_position.y << " " << reading[i].predicted_position.z << " " << reading[i].position.w << '\n';
    // std::cout << reading[i].position.x << " " << reading[i].position.y << " " << reading[i].position.z << " " << reading[i].position.w << '\n';
    std::cout << '\n';
  }

  std::cout << '\n';

}


void FluidSystem::print_density(CommandPool& commandpool, VkPhysicalDevice pysical_device) {
  vkDeviceWaitIdle(device);
  VkDeviceSize size = sizeof(uint32_t) * 17576; 
  HostBuffer staging(
    device, 
    physical_device, 
    size, 
    VK_BUFFER_USAGE_TRANSFER_DST_BIT
  );

  staging.copyBuffer(*spatial_lookup_buffer, commandpool);
  std::vector<uint32_t> values(17576);
  staging.getData(values.data());

  for (size_t i = 0; i < values.size(); i++) {
    if (values[i] != std::numeric_limits<uint32_t>::max()) {
      std::cout << values[i] << " " << i << '\n';;
    }

  }
  std::cout << '\n';
}

