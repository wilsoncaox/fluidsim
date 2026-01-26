
#pragma once

#include "Entity.hpp"
#include "Camera.hpp"
#include "../system/FluidSystem.hpp"

#include <vector>
#include <memory>

struct Scene {
  inline static uint32_t instances = 20000;

  void init(VkDevice device, VkPhysicalDevice physical_device);

  void update(Window& window, double delta_time);

  std::vector<std::unique_ptr<Entity>> entities;
  std::unique_ptr<FluidSystem> system;
  std::unique_ptr<Camera> camera;
};
