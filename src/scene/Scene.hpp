
#pragma once

#include "../system/FluidSystem.hpp"
#include "../context/VulkanContext.hpp"
#include "Camera.hpp"
#include "Entity.hpp"

#include <memory>

struct Scene {
  inline static uint32_t instances = 10000;

  void init(VulkanContext& context, DescriptorBuilder& builder);
  // void update(Window& window, double delta_time);

  std::vector<std::unique_ptr<Entity>> entities;
  std::unique_ptr<FluidSystem> fluid_system;
  std::unique_ptr<Camera> camera;
  std::unique_ptr<CommandPool> commandpool;

};
