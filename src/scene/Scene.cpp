
#include "Scene.hpp"
#include "../system/FluidSystem.hpp"
#include "Camera.hpp"
#include "entities/Sphere.hpp"

#include <memory>
#include <iostream>

void Scene::init(VulkanContext& context, DescriptorBuilder& builder) {
  fluid_system = std::make_unique<FluidSystem>(context.device, context.physical_device, builder, Scene::instances); 
  fluid_system->init_data(context.get_commandpool(), context.physical_device);

  camera = std::make_unique<Camera>(context.device, context.physical_device, builder);

  entities.emplace_back(std::make_unique<Sphere>());
}

