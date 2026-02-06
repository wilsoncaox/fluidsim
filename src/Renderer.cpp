
#include "Renderer.hpp"

#include <stdexcept>
#include <cmath>
#include <iostream>
#include <vulkan/vulkan_core.h>

void Renderer::init(VulkanContext& context, Window& window) {
  device = context.device;
  physical_device = context.physical_device;
  queue = context.queue;

  commandbuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++) {
    context.get_commandpool().create_command_buffer(&commandbuffers[i], 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  }

  swapchain = std::make_unique<Swapchain>(context);
  swapchain->create(window);

  renderpass = std::make_unique<Renderpass>(context.device, context.physical_device, swapchain->format.format, swapchain->find_depth_format());  
  renderpass->init_resources(*swapchain, context.get_commandpool());


  graphics_pipeline = std::make_unique<GraphicsPipeline>(device, "shaders/vertex.vert.spv", "shaders/vertex.frag.spv");
}

void Renderer::build_resources(VulkanContext& context, Scene& scene) {
  meshes.reserve(scene.entities.size());
  for (auto& entity : scene.entities) {
    meshes.emplace_back(context.device, context.physical_device, *entity, context.get_commandpool(), Scene::instances); 
  }
 
  graphics_pipeline->create(renderpass->renderpass, {scene.fluid_system->particle_layout_graphics, scene.camera->layout});
}

void Renderer::recreate_frame(VulkanContext& context, Window& window) {
  window.wait_events();
  vkDeviceWaitIdle(device);

  renderpass->clean_resources();
  swapchain->cleanup();
  swapchain->create(window);
  renderpass->init_resources(*swapchain, context.get_commandpool());
}

void Renderer::draw(VulkanContext& context, Window& window, Scene& scene, uint32_t current_frame) {

  uint32_t image_index;
  VkResult result = swapchain->acquire_image(&image_index, current_frame);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreate_frame(context, window);
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }


  vkResetCommandBuffer(commandbuffers[current_frame], 0);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandbuffers[current_frame], &begin_info) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  scene.fluid_system->run(context.get_commandpool(), commandbuffers[current_frame]);

  renderpass->begin_renderpass(*swapchain, commandbuffers[current_frame], image_index);
  graphics_pipeline->bind_pipeline(commandbuffers[current_frame]);

  scene.fluid_system->bind_particle(commandbuffers[current_frame], *graphics_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
  scene.camera->bind_camera(commandbuffers[current_frame], *graphics_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);

  for (auto& mesh : meshes) {
    mesh.bind(commandbuffers[current_frame]);
    mesh.draw(commandbuffers[current_frame]);
  }

  renderpass->end_renderpass(commandbuffers[current_frame]);

   
  if (vkEndCommandBuffer(commandbuffers[current_frame]) != VK_SUCCESS) {
    throw std::runtime_error("Unable to end command buffer");
  }

  result = swapchain->submit_command(commandbuffers[current_frame], current_frame, &image_index);
  // scene.fluid_system->print_data(context.get_commandpool(), context.physical_device);
  // scene.fluid_system->print_density(context.get_commandpool(), context.physical_device);
  // std::cout <<   " ======= "<< '\n';

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.resized) {
    window.resized = false;
    recreate_frame(context, window);
    return;
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
}

