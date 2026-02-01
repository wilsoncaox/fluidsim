
#pragma once

#include "Renderer.hpp"

class Engine {

  public:
    Engine();


    void run(); 

  private:
    void init_world();

    void display_frames(int* frames, float delta_time);

    Window window;
    VulkanContext context;

    DescriptorHandler handler;

    Scene scene; 

    Renderer renderer;
    

    uint32_t current_frame = 0;
};
