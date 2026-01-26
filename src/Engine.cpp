
#include "Engine.hpp"


#include "Renderer.hpp"
#include "descriptors/sets/CameraDescriptorSet.hpp"
#include "descriptors/sets/ParticleDensityDescriptorSet.hpp"
#include "descriptors/sets/ParticleDescriptorSet.hpp"
#include "descriptors/sets/ParticlePositionDescriptorSet.hpp"

#include <iostream>

Engine::Engine() {
  context.init(window);

  scene.init(context.device, context.physical_device);
  renderer.init(context, window);
  renderer.build_meshes(context, scene);
  renderer.build_resources(context, scene);
  handler.init(context.device);

  init_resources(scene);
}

void Engine::run() {
  double last_time = window.time();
  int frames = 0;
  double frame_time = window.time();

  while (!window.should_window_close()) {
    double current_time = window.time();
    float delta_time = static_cast<float>(current_time - last_time);

    scene.update(window, delta_time);
    
    renderer.draw(window, scene, handler, current_frame);

    current_frame = (current_frame + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
    window.poll_events();

    frames++;
    if (current_time - frame_time >= 1.0) {
      double fps = frames / delta_time;
      std::string title = "My App - FPS | " + std::to_string((int)fps);

      window.set_title(title);
      frames = 0;
      frame_time = current_time;
    }

    last_time = current_time; 
  }
}

void Engine::init_resources(Scene& scene) {
  std::vector<VkDescriptorSet> particle_sets(Swapchain::MAX_FRAMES_IN_FLIGHT);
  VkDescriptorSetLayout particle_layout;

  auto system = scene.system.get();

  // Need to use different buffer for graphics and compute
  for (size_t i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++) {
    auto descriptors = system->get_resource();
    handler.bind_descriptor(descriptors[i]);
    handler.build_descriptor(particle_sets[i], particle_layout);
    handler.clear_descriptor();
  }

  std::vector<VkDescriptorSet> camera_sets(Swapchain::MAX_FRAMES_IN_FLIGHT);
  VkDescriptorSetLayout camera_layout;
  
  // Using same buffer for every frame

  handler.bind_descriptor(scene.camera->get_resource()); 
  for (size_t i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++) {
    handler.build_descriptor(camera_sets[i], camera_layout);
  }
  

  // Pipelines for sorting
  // renderer.histogram = std::make_unique<ComputePipeline>(context.device, "shaders/vertex.histogram.comp");
  // renderer.histogram->create({particle_layout}, {{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)}});


  VkPipelineLayout graphics_layout = renderer.add_graphics_pipeline(
    context, 
    "shaders/vertex.vert.spv", 
    "shaders/vertex.frag.spv", 
    {camera_layout, particle_layout}
  );  


  renderer.particle_sim_pipeline = std::make_unique<ComputePipeline>(context.device, "shaders/vertex.comp.spv");
  renderer.particle_sim_pipeline->create({particle_layout, particle_layout});

  renderer.pre_calc_position_pipeline = std::make_unique<ComputePipeline>(context.device, "shaders/vertex.position.comp.spv");
  renderer.pre_calc_position_pipeline->create({particle_layout, particle_layout});

  renderer.pre_calc_density_pipeline = std::make_unique<ComputePipeline>(context.device, "shaders/vertex.density.comp.spv");
  renderer.pre_calc_density_pipeline->create({particle_layout, particle_layout});

  handler.compute_particle_position_read = std::make_unique<ParticleDensityDescriptorSet>(
    renderer.pre_calc_position_pipeline->get_pipeline_layout(), 
    VK_PIPELINE_BIND_POINT_COMPUTE, 
    system->get_buffers(), 
    particle_sets, 
    0, 
    1
  );


  handler.compute_particle_position_write = std::make_unique<ParticleDensityDescriptorSet>(
    renderer.pre_calc_position_pipeline->get_pipeline_layout(), 
    VK_PIPELINE_BIND_POINT_COMPUTE, 
    system->get_buffers(), 
    particle_sets, 
    1, 
    0
  );


  handler.compute_particle_density_read = std::make_unique<ParticleDensityDescriptorSet>(
    renderer.pre_calc_density_pipeline->get_pipeline_layout(), 
    VK_PIPELINE_BIND_POINT_COMPUTE, 
    system->get_buffers(), 
    particle_sets, 
    1, 
    1
  );

  handler.compute_particle_density_write = std::make_unique<ParticleDensityDescriptorSet>(
    renderer.pre_calc_density_pipeline->get_pipeline_layout(), 
    VK_PIPELINE_BIND_POINT_COMPUTE, 
    system->get_buffers(), 
    particle_sets, 
    2, 
    0
  );

  handler.compute_particle_read = std::make_unique<ParticleDescriptorSet>(
    renderer.particle_sim_pipeline->get_pipeline_layout(), 
    VK_PIPELINE_BIND_POINT_COMPUTE, 
    system->get_buffers(), 
    particle_sets, 
    2, 
    1
  );

  handler.compute_particle_write = std::make_unique<ParticleDescriptorSet>(
    renderer.particle_sim_pipeline->get_pipeline_layout(), 
    VK_PIPELINE_BIND_POINT_COMPUTE, 
    system->get_buffers(), 
    particle_sets, 
    3, 
    0
  );
  handler.graphics_particle_set = std::make_unique<ParticleDescriptorSet>(
    graphics_layout, 
    VK_PIPELINE_BIND_POINT_GRAPHICS, 
    system->get_buffers(), 
    particle_sets, 
    3, 
    1
  );

  handler.graphics_camera_set = std::make_unique<CameraDescriptorSet>(
    graphics_layout, 
    camera_sets, 
    0
  ); 
}

