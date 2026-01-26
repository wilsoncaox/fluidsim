
#include "Renderer.hpp"

#include <stdexcept>
#include <cmath>
#include <iostream>

void Renderer::init(VulkanContext& context, Window& window) {
  device = context.device;
  physical_device = context.physical_device;
  queue = context.queue;

  commandpool = std::make_unique<CommandPool>(context.device, context.queue, context.queue_index.index.value()); 
  commandbuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++) {
    commandpool->create_command_buffer(&commandbuffers[i], 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  }

  swapchain = std::make_unique<Swapchain>(context);
  swapchain->create(window);

  renderpass = std::make_unique<Renderpass>(context.device, context.physical_device, swapchain->format.format, swapchain->find_depth_format());  
  renderpass->init_resources(*swapchain, *commandpool);

}

VkPipelineLayout Renderer::add_graphics_pipeline(VulkanContext& context, std::string vertex, std::string fragment, std::vector<VkDescriptorSetLayout> graphic_layout) {
  std::unique_ptr<GraphicsPipeline> graphics_pipeline = std::make_unique<GraphicsPipeline>(context.device, vertex, fragment);
  graphics_pipeline->create(renderpass->renderpass, graphic_layout);
  VkPipelineLayout result = graphics_pipeline->get_pipeline_layout();
  graphics_pipelines.push_back(std::move(graphics_pipeline));

  return result;
}


void Renderer::build_meshes(VulkanContext& context, Scene& scene) {
  meshes.reserve(scene.entities.size());
  for (auto& entity : scene.entities) {
    meshes.emplace_back(context.device, context.physical_device, *entity, *commandpool, Scene::instances); 
  }
}

void Renderer::recreate_frame(Window& window) {
  window.wait_events();
  vkDeviceWaitIdle(device);

  renderpass->clean_resources();
  swapchain->cleanup();
  swapchain->create(window);
  renderpass->init_resources(*swapchain, *commandpool);
}

void Renderer::draw(Window& window, Scene& scene, DescriptorHandler& handler, uint32_t current_frame) {

  uint32_t image_index;
  VkResult result = swapchain->acquire_image(&image_index, current_frame);

  auto system = scene.system.get();
  system->print_data(*commandpool, physical_device, (current_frame*3 + 3) % (Swapchain::MAX_FRAMES_IN_FLIGHT));

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreate_frame(window);
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


  handler.compute_particle_position_read->bind(commandbuffers[current_frame], current_frame);
  handler.compute_particle_position_write->bind(commandbuffers[current_frame], current_frame);
  pre_calc_position_pipeline->bind_pipeline(commandbuffers[current_frame]);
  vkCmdDispatch(commandbuffers[current_frame], std::ceil(Scene::instances / 256) + 1, 1, 1);

  VkBufferMemoryBarrier position_barrier{};  
  position_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER; 
  position_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  position_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  position_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  position_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  position_barrier.buffer = handler.compute_particle_position_write->get_buffer(current_frame);
  position_barrier.offset = 0;
  position_barrier.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(
    commandbuffers[current_frame], 
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    0,
    0, nullptr,
    1, &position_barrier,
    0, nullptr
  );


  

  handler.compute_particle_density_read->bind(commandbuffers[current_frame], current_frame);
  handler.compute_particle_density_write->bind(commandbuffers[current_frame], current_frame);

  pre_calc_density_pipeline->bind_pipeline(commandbuffers[current_frame]);
  vkCmdDispatch(commandbuffers[current_frame], std::ceil(Scene::instances / 256) + 1, 1, 1);

  VkBufferMemoryBarrier density_barrier{};  
  density_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER; 
  density_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  density_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  density_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  density_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  density_barrier.buffer = handler.compute_particle_density_write->get_buffer(current_frame);
  density_barrier.offset = 0;
  density_barrier.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(
    commandbuffers[current_frame], 
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    0,
    0, nullptr,
    1, &density_barrier,
    0, nullptr
  );

  handler.compute_particle_read->bind(commandbuffers[current_frame], current_frame);
  handler.compute_particle_write->bind(commandbuffers[current_frame], current_frame);
  particle_sim_pipeline->bind_pipeline(commandbuffers[current_frame]); 
  vkCmdDispatch(commandbuffers[current_frame], std::ceil(Scene::instances / 256) + 1, 1, 1);

  VkBufferMemoryBarrier particle_barrier{};  
  particle_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER; 
  particle_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  particle_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  particle_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  particle_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  particle_barrier.buffer = handler.compute_particle_write->get_buffer(current_frame);
  particle_barrier.offset = 0;
  particle_barrier.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(
    commandbuffers[current_frame], 
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
    0,
    0, nullptr,
    1, &particle_barrier,
    0, nullptr
  );

  renderpass->begin_renderpass(*swapchain, commandbuffers[current_frame], image_index);

  handler.graphics_camera_set->bind(commandbuffers[current_frame], current_frame);
  handler.graphics_particle_set->bind(commandbuffers[current_frame], current_frame);

  for (auto& pipeline : graphics_pipelines) {
    pipeline->bind_pipeline(commandbuffers[current_frame]);
  }

  for (auto& mesh : meshes) {
    mesh.bind(commandbuffers[current_frame]);
    mesh.draw(commandbuffers[current_frame]);
  }

  renderpass->end_renderpass(commandbuffers[current_frame]);

  if (vkEndCommandBuffer(commandbuffers[current_frame]) != VK_SUCCESS) {
    throw std::runtime_error("Unable to end command buffer");
  }
  
  result = swapchain->submit_command(commandbuffers[current_frame], current_frame, &image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.resized) {
    window.resized = false;
    recreate_frame(window);
    return;
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
}

void Renderer::build_resources(VulkanContext& context, Scene& scene) {
  scene.system->init_data(*commandpool, context.physical_device);
}
