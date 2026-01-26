
#include "FluidSystem.hpp"
#include "../buffer/HostBuffer.hpp"

#include <random>
#include <iostream>
#include <cmath>

FluidSystem::FluidSystem(
  VkDevice device, 
  VkPhysicalDevice physical_device, 
  uint32_t buffer_count, 
  uint32_t instance_count
) : System(device), instance_count(instance_count) {
  buffers.reserve(buffer_count);  
  resources.reserve(buffer_count);

  for (size_t i = 0; i < buffer_count; i++) {
    buffers.emplace_back(
        device, 
        physical_device, 
        sizeof(FluidData)*instance_count, 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    resources.emplace_back(std::make_unique<StorageBuffer>(device, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT));
    resources[i]->bind(&buffers[i]);
  }
};

std::vector<Descriptor*> FluidSystem::get_resource() const {
  std::vector<Descriptor*> descriptors;
  for (auto& resource : resources) {
    descriptors.push_back(resource.get());
  }

  return descriptors;
}

std::vector<VkBuffer> FluidSystem::get_buffers() {
  std::vector<VkBuffer> buffer_resources;
  for (auto& buffer : buffers) {
    buffer_resources.push_back(buffer.buffer);
  }

  return buffer_resources;
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

  staging.fillData(values.data(), size);

  for (auto& buffer : buffers) {
    buffer.copyBuffer(staging, commandpool);
  }
}


void FluidSystem::print_data(CommandPool& commandpool, VkPhysicalDevice physical_device, uint32_t frame) {
  VkDeviceSize size = sizeof(FluidData) * instance_count; 
  HostBuffer staging(
    device, 
    physical_device, 
    size, 
    VK_BUFFER_USAGE_TRANSFER_DST_BIT
  );

  staging.copyBuffer(buffers[frame], commandpool);

  std::vector<FluidData> values(instance_count);
  staging.getData(values.data());
  
  auto value = values[5000];
  std::cout << "position: "<< value.position.x << " " << value.position.y << " " << value.position.z << '\n';
  std::cout << "velocity: " << value.velocity.x << " " << value.velocity.y << " " << value.velocity.z << '\n';
  std::cout << "density: " << value.density.x << '\n';
  std::cout << "predicted position: "<< value.predicted_position.x << " " << value.predicted_position.y << " " << value.predicted_position.z << '\n';
  std::cout << "random: " << value.density.y << " " << value.density.z << " " << value.density.w << '\n';
  std::cout << '\n';
    
}
