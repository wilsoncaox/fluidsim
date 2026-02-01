
#include "Engine.hpp"


#include "Renderer.hpp"
#include <iostream>

Engine::Engine() {
  context.init(window);
  renderer.init(context, window);
  handler.init(context.device);
  scene.init(context, handler.descriptor_builder);

  renderer.build_resources(context, scene);
}

void Engine::run() {
  double last_time = window.time();
  double frame_time = window.time();
  int frames = 0;

  while (!window.should_window_close()) {
    double current_time = window.time();
    float delta_time = static_cast<float>(current_time - last_time);

    scene.camera->update(window, delta_time);

    renderer.draw(context, window, scene, current_frame);

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
