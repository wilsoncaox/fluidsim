
#include "Sort.hpp"
#include "../../buffer/HostBuffer.hpp"
#include "../FluidSystem.hpp"
#include <vulkan/vulkan_core.h>
#include <array>
#include <iostream>

Sort::Sort(
  VkDevice device, 
  VkPhysicalDevice physical_device, 
  uint32_t count, 
  uint32_t size
) : device(device), physical_device(physical_device), data_count(count), data_size(size) {

  data_temp.reserve(2);
  data_temp_set.reserve(2);
  for (size_t i = 0; i < 2; i++) {
    data_temp.emplace_back(
      device, 
      physical_device, 
      data_count*data_size, 
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    ); 
  }

  keys = std::make_unique<Buffer>(
    device, 
    physical_device, 
    data_size*data_count, 
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  ); 

  offset = std::make_unique<Buffer>(
    device, 
    physical_device, 
    sizeof(uint32_t), 
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  ); 

  scans.reserve(4);
  scan_set.resize(4);
  for (size_t i = 0; i < 4; i++) {
    scans.emplace_back(
      device, 
      physical_device, 
      sizeof(uint32_t)*data_count, 
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
  }
}

void Sort::init(DescriptorBuilder& handler, VkDescriptorSetLayout data_layout, uint32_t x, uint32_t y, uint32_t z) {
  
  groupCountX = x;
  groupCountY = y;
  groupCountZ = z;


  handler.clear();

  handler.bind_buffer(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, keys->get_info()); 
  handler.build(key_set, key_layout);
  handler.clear();

  // Temp buffer for moving data
  handler.bind_buffer(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, data_temp[0].get_info()); 
  handler.build(data_temp_set[0], data_temp_layout);
  handler.clear();

  handler.bind_buffer(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, data_temp[1].get_info()); 
  handler.build(data_temp_set[1], data_temp_layout);
  handler.clear();

  // Binding offset
  // set 0 for read, set 1 for write
  handler.bind_buffer(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, offset->get_info()); 
  handler.build(offset_set, offset_layout);
  handler.clear();  


  // Binding scans
  // For now since we using two digits 0 and 1 we have four
  // 0 takes the first two buffers 1 takes the next two
  for (size_t i = 0; i < scan_set.size(); i++) {
    handler.bind_buffer(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, scans[i].get_info());
    handler.build(scan_set[i], scan_layout);
    handler.clear();  
  }
 
  // push constant for index 
   

  VkPushConstantRange constant{};
  constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  constant.size = sizeof(PushConstant);
  constant.offset = 0;


  key_pipeline = std::make_unique<ComputePipeline>(device, "shaders/vertex.key.comp.spv");
  key_pipeline->create({key_layout, data_temp_layout, scan_layout, scan_layout, scan_layout, scan_layout}, {constant});

  // Calculating offset from histogram 
  offset_pipeline = std::make_unique<ComputePipeline>(device, "shaders/vertex.offset.comp.spv");   
  offset_pipeline->create({data_temp_layout, offset_layout}, {constant});


  // Changing digits into 1 and 0's
  digits_pipeline = std::make_unique<ComputePipeline>(device, "shaders/vertex.digits.comp.spv");
  digits_pipeline->create({scan_layout}, {constant});

  // exlcusive prefix scan for digits
  scanning_pipeline = std::make_unique<ComputePipeline>(device, "shaders/vertex.scan.comp.spv");  
  scanning_pipeline->create({scan_layout, scan_layout}, {constant});

  sorting_pipeline = std::make_unique<ComputePipeline>(device, "shaders/vertex.sort.comp.spv");  
  sorting_pipeline->create({data_temp_layout, data_temp_layout, scan_layout, scan_layout, offset_layout}, {constant});
}

void Sort::init_temp(VkCommandBuffer commandbuffer, VkBuffer initial, CommandPool& commandpool) {
  // Copies inital data in temp data at index 0
  VkBufferCopy data_region{};
  data_region.size = data_count*data_size;
  vkCmdCopyBuffer(commandbuffer, initial, data_temp[0].buffer, 1, &data_region);

  VkBufferMemoryBarrier data_copy_barrier{};
  data_copy_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  data_copy_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  data_copy_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  data_copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  data_copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  data_copy_barrier.buffer = data_temp[0].buffer;
  data_copy_barrier.offset = 0;
  data_copy_barrier.size = VK_WHOLE_SIZE;


  vkCmdPipelineBarrier(
    commandbuffer, 
    VK_PIPELINE_STAGE_TRANSFER_BIT, 
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
    0, 
    0, nullptr, 
    1, &data_copy_barrier, 
    0, nullptr
  );
}

void Sort::extract_key(VkCommandBuffer commandbuffer, uint32_t data_index) {
    VkBufferCopy data_region{};
    data_region.size = data_count*data_size;
    vkCmdCopyBuffer(commandbuffer, data_temp[data_index].buffer, keys->buffer, 1, &data_region);

    VkBufferMemoryBarrier data_copy_barrier{};
    data_copy_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    data_copy_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    data_copy_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    data_copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    data_copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    data_copy_barrier.buffer = keys->buffer;
    data_copy_barrier.offset = 0;
    data_copy_barrier.size = VK_WHOLE_SIZE;


    vkCmdPipelineBarrier(
      commandbuffer, 
      VK_PIPELINE_STAGE_TRANSFER_BIT, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      0, 
      0, nullptr, 
      1, &data_copy_barrier, 
      0, nullptr
    );

    std::array<VkDescriptorSet, 6> key_sets = { 
      key_set, 
      data_temp_set[data_index],
      scan_set[0],
      scan_set[1],
      scan_set[2],
      scan_set[3],
    };
    key_pipeline->bind_descriptor_sets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 0, static_cast<uint32_t>(key_sets.size()), key_sets.data());
    key_pipeline->bind_pipeline(commandbuffer);
    key_pipeline->bind_push_constants(commandbuffer, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t), &data_count);
    vkCmdDispatch(commandbuffer, groupCountX, groupCountY, groupCountZ);

    std::vector<VkBufferMemoryBarrier> barriers;
    VkBufferMemoryBarrier key_barrier{};
    key_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    key_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    key_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    key_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    key_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    key_barrier.buffer = data_temp[data_index].buffer;
    key_barrier.offset = 0;
    key_barrier.size = VK_WHOLE_SIZE;

    barriers.push_back(key_barrier);

    for (size_t i = 0; i < scans.size(); i++) {
      VkBufferMemoryBarrier scan_barrier{};
      scan_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
      scan_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      scan_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      scan_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      scan_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      scan_barrier.buffer = scans[i].buffer;
      scan_barrier.offset = 0;
      scan_barrier.size = VK_WHOLE_SIZE;
    
      barriers.push_back(scan_barrier);
    }

    vkCmdPipelineBarrier(
      commandbuffer, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      0, 
      0, nullptr, 
      static_cast<uint32_t>(barriers.size()), barriers.data(), 
      0, nullptr
    );
}

void Sort::reset_offset(VkCommandBuffer commandbuffer) {
    vkCmdFillBuffer(
      commandbuffer,
      offset->buffer,
      0,
      sizeof(uint32_t),
      0
    );

    VkBufferMemoryBarrier offset_reset_barrier{};
    offset_reset_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    offset_reset_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    offset_reset_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    offset_reset_barrier.buffer = offset->buffer;
    offset_reset_barrier.offset = 0;
    offset_reset_barrier.size = sizeof(uint32_t);

    vkCmdPipelineBarrier(
      commandbuffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
      0,
      0, nullptr,
      1, &offset_reset_barrier,
      0, nullptr
    );
}

void Sort::find_offset(VkCommandBuffer commandbuffer, uint32_t index, uint32_t data_index) {
    PushConstant constant = {data_count, index, 0};

    std::array<VkDescriptorSet, 2> offset_sets = { data_temp_set[data_index], offset_set };
    offset_pipeline->bind_descriptor_sets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 0, static_cast<uint32_t>(offset_sets.size()), offset_sets.data());
    offset_pipeline->bind_push_constants(commandbuffer, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t)*2, &constant);
    offset_pipeline->bind_pipeline(commandbuffer);
    vkCmdDispatch(commandbuffer, groupCountX, groupCountY, groupCountZ);

    VkBufferMemoryBarrier offset_barrier{};
    offset_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    offset_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    offset_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    offset_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    offset_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    offset_barrier.buffer = offset->buffer;
    offset_barrier.offset = 0;
    offset_barrier.size = VK_WHOLE_SIZE;

    vkCmdPipelineBarrier(
      commandbuffer, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      0, 
      0, nullptr, 
      1, &offset_barrier, 
      0, nullptr
    );

}

void Sort::digit_scans(VkCommandBuffer commandbuffer, uint32_t index) {
  uint32_t digit_count = 2;
  digits_pipeline->bind_pipeline(commandbuffer); 

  // First loop for 0's and 1's
  for (uint32_t i = 0; i < digit_count; i++) {
    PushConstant constant = {data_count, index, i};
    digits_pipeline->bind_push_constants(commandbuffer, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t)*3, &constant);

    // Second loop for read and write
    for (uint32_t j = i; j < digit_count + i; j++) {
      digits_pipeline->bind_descriptor_sets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 0, 1, &scan_set[i + j]);
      vkCmdDispatch(commandbuffer, groupCountX, groupCountY, groupCountZ);
    }
  }

  std::vector<VkBufferMemoryBarrier> digit_barriers;
  for (size_t i = 0; i < scans.size(); i++) {
    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = scans[i].buffer;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;

    digit_barriers.push_back(barrier);
  }

  vkCmdPipelineBarrier(
      commandbuffer, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      0, 
      0, nullptr, 
      static_cast<uint32_t>(digit_barriers.size()), digit_barriers.data(), 
      0, nullptr
      );
}

size_t Sort::dispatch_scan(VkCommandBuffer commandbuffer, std::array<VkDescriptorSet, 2> sets, std::array<VkBuffer, 2> buffers) {
  size_t read = 0; 
  size_t write = 1;

  for (size_t i = 1; i < data_count; i <<= 1) {

    PushConstant constant = {data_count, static_cast<uint32_t>(i)};
    std::array<VkDescriptorSet, 2> set_buffering = {sets[read], sets[write]}; 
    scanning_pipeline->bind_push_constants(commandbuffer, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t)*2, &constant);
    scanning_pipeline->bind_descriptor_sets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 0, static_cast<uint32_t>(set_buffering.size()), set_buffering.data());
    vkCmdDispatch(commandbuffer, groupCountX, groupCountY, groupCountZ);

    VkBufferMemoryBarrier scanning_barrier{};
    scanning_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    scanning_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    scanning_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    scanning_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    scanning_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    scanning_barrier.buffer = buffers[write];
    scanning_barrier.offset = 0;
    scanning_barrier.size = VK_WHOLE_SIZE;

    vkCmdPipelineBarrier(
      commandbuffer, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      0, 
      0, nullptr, 
      1, &scanning_barrier,
      0, nullptr
    );

    read = (read + 1) % 2;
    write = (write + 1) % 2;
  }

  return read;
}


void Sort::partition_sort(VkCommandBuffer commandbuffer, size_t index, size_t read_index, size_t write_index, size_t zero_index, size_t one_index) {
    std::array<VkDescriptorSet, 5> sets = { 
      data_temp_set[read_index], 
      data_temp_set[write_index], 
      scan_set[zero_index], 
      scan_set[one_index + 2], 
      offset_set,
    };

    PushConstant constant = {data_count, static_cast<uint32_t>(index)};

    sorting_pipeline->bind_pipeline(commandbuffer);
    sorting_pipeline->bind_descriptor_sets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 0, static_cast<uint32_t>(sets.size()), sets.data());
    sorting_pipeline->bind_push_constants(commandbuffer, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t)*2, &constant);
    vkCmdDispatch(commandbuffer, groupCountX, groupCountY, groupCountZ);


    VkBufferMemoryBarrier sorting_barrier{};
    sorting_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    sorting_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    sorting_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sorting_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    sorting_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    sorting_barrier.buffer = data_temp[write_index].buffer;
    sorting_barrier.offset = 0;
    sorting_barrier.size = VK_WHOLE_SIZE;

    vkCmdPipelineBarrier(
      commandbuffer, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      0, 
      0, nullptr, 
      1, &sorting_barrier,
      0, nullptr
    );
}

void Sort::final_fill(VkCommandBuffer commandbuffer, VkBuffer src, VkBuffer dst) {
  VkBufferCopy final_region{};
  final_region.size = data_count*data_size;
  vkCmdCopyBuffer(commandbuffer, src, dst, 1, &final_region);

  VkBufferMemoryBarrier fill_barrier{};
  fill_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  fill_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  fill_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  fill_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  fill_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  fill_barrier.buffer = dst;
  fill_barrier.offset = 0;
  fill_barrier.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(
      commandbuffer, 
      VK_PIPELINE_STAGE_TRANSFER_BIT, 
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
      0, 
      0, nullptr, 
      1, &fill_barrier,
      0, nullptr
  );
}

void Sort::run(
  CommandPool& commandpool, 
  VkCommandBuffer commandbuffer, 
  VkBuffer data_buffer
) {
  size_t data_read_index = 0;
  size_t data_write_index = 1;
  init_temp(commandbuffer, data_buffer, commandpool);

  for (uint32_t index = 0; index < 32; index++) {
    extract_key(commandbuffer, data_read_index);
    reset_offset(commandbuffer);

    find_offset(commandbuffer, index, data_read_index);


    digit_scans(commandbuffer, index);

    scanning_pipeline->bind_pipeline(commandbuffer);
    size_t zeroes_index = dispatch_scan(commandbuffer, {scan_set[0], scan_set[1]}, {scans[0].buffer, scans[1].buffer});
    size_t ones_index = dispatch_scan(commandbuffer, {scan_set[2], scan_set[3]}, {scans[2].buffer, scans[3].buffer});

    partition_sort(commandbuffer, index, data_read_index, data_write_index, zeroes_index, ones_index);

    data_read_index = (data_read_index + 1) % 2;
    data_write_index = (data_write_index + 1) % 2;
  }

  final_fill(commandbuffer, data_temp[data_read_index].buffer, data_buffer);


  // vkDeviceWaitIdle(device);
  // VkDeviceSize size = sizeof(FluidData) * data_count; 
  // HostBuffer staging(
  //   device, 
  //   physical_device, 
  //   size, 
  //   VK_BUFFER_USAGE_TRANSFER_DST_BIT
  // );
  //
  // staging.copyBuffer(data_temp[data_read_index], commandpool);
  //
  // std::vector<FluidData> values(data_count);
  // staging.getData(values.data());
  //
  // for (size_t i = 0; i < data_count; i++) {
  //   auto value = values[i];
  //   std::cout << value.position.x << " " << value.position.y << " " << value.position.z << " " << value.position.w << '\n';
  //
  // }


  // vkDeviceWaitIdle(device);
  // VkDeviceSize size = sizeof(uint32_t) * 1; 
  // HostBuffer staging(
  //   device, 
  //   physical_device, 
  //   size, 
  //   VK_BUFFER_USAGE_TRANSFER_DST_BIT
  // );
  //
  // staging.copyBuffer(*offset, commandpool);
  //
  // std::vector<uint32_t> values(1);
  // staging.getData(values.data());
  // std::cout << values[0] << '\n';
  //
  // std::cout << '\n';

}

