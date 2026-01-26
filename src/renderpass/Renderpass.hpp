#pragma once

#include "Swapchain.hpp"
#include "../image/Image.hpp"

#include <memory>

class Renderpass {

  public:
    Renderpass(VkDevice device, VkPhysicalDevice physical_device, VkFormat surface_format, VkFormat depth_format);
    ~Renderpass();
    
    void init_resources(Swapchain& swapchain, CommandPool& command_pool);
    void clean_resources();

    void begin_renderpass(Swapchain& swapchain, VkCommandBuffer command_buffer, uint32_t image_index);
    void end_renderpass(VkCommandBuffer command_buffer); 


    VkRenderPass renderpass; 
  private:
    void create_depth_resource(Swapchain& swapchain, CommandPool& command_pool);
    void create_framebuffers(Swapchain& swapchain); 

    VkDevice device; 
    VkPhysicalDevice physical_device;

    std::vector<VkFramebuffer> framebuffers;
    std::unique_ptr<Image> depth_image;

};
