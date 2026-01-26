
#include "Scene.hpp"
#include "entities/Sphere.hpp"
#include "../system/FluidSystem.hpp"
#include "../renderpass/Swapchain.hpp"

void Scene::init(VkDevice device, VkPhysicalDevice physical_device) {
  system = std::make_unique<FluidSystem>(device, physical_device, Swapchain::MAX_FRAMES_IN_FLIGHT, Scene::instances); 
  camera = std::make_unique<Camera>(device, physical_device, 0);
  entities.emplace_back(std::make_unique<Sphere>());
}

void Scene::update(Window& window, double delta_time) {

  camera->update(window, delta_time);
}

