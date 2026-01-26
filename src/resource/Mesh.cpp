#include "Mesh.hpp"

#include "../buffer/HostBuffer.hpp"
#include <iostream>

Mesh::Mesh(
  VkDevice device, 
  VkPhysicalDevice physical_device, 
  Entity& entity, 
  CommandPool& command_pool,
  uint32_t instances
) : device(device), physical_device(physical_device), instances(instances) {
  create_vertex_buffer(entity, command_pool);
  create_index_buffer(entity, command_pool);
}

void Mesh::create_vertex_buffer(Entity& entity, CommandPool& command_pool) {
  VkDeviceSize size = sizeof(entity.vertices[0]) * entity.vertices.size();

  HostBuffer staging(
    device, 
    physical_device, 
    size, 
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT
  );

  staging.fillData(entity.vertices.data(), size); 

  vertex_buffer = std::make_unique<Buffer>(
    device, 
    physical_device, 
    size, 
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );

  vertex_buffer->copyBuffer(staging, command_pool);
}

void Mesh::create_index_buffer(Entity& entity, CommandPool& command_pool) {
  VkDeviceSize size = sizeof(entity.indices[0]) * entity.indices.size();
  index_count = static_cast<uint32_t>(entity.indices.size());

  HostBuffer staging(
    device, 
    physical_device, 
    size, 
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT
  );
  staging.fillData(entity.indices.data(), size); 

  index_buffer = std::make_unique<Buffer>(
    device, 
    physical_device, 
    size, 
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );

  index_buffer->copyBuffer(staging, command_pool);
}

void Mesh::bind(VkCommandBuffer commandbuffer) {
  VkBuffer vertex_buffers[] = {vertex_buffer->buffer};
  VkDeviceSize offsets[] = {0};

  vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertex_buffers, offsets);
  vkCmdBindIndexBuffer(commandbuffer, index_buffer->buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Mesh::draw(VkCommandBuffer commandbuffer) {
  vkCmdDrawIndexed(commandbuffer, index_count, instances, 0, 0, 0);
}
