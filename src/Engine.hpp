
#pragma once

#include "Renderer.hpp"

class Engine {

  public:
    Engine();


    void run(); 

  private:
    void init_world();
    void init_resources(Scene& scene);

    void display_frames(int* frames, float delta_time);

    Window window;
    VulkanContext context;

    Scene scene;

    DescriptorHandler handler;
    Renderer renderer;

    uint32_t current_frame = 0;
};
