#pragma once

#include "../buffer/Buffer.hpp"

class Image {

  public:
      Image(
        VkDevice device, 
        VkPhysicalDevice physical_device,
        uint32_t width, uint32_t height, 
        VkFormat format, 
        VkImageUsageFlags usage, 
        VkImageAspectFlags aspect, 
        VkMemoryPropertyFlags property
      );

      void transition_image_layout(CommandPool& command_pool, VkImageLayout old_layout, VkImageLayout new_layout);
      void copy_from_buffer(CommandPool& command_pool, Buffer& buffer); 
      bool has_stencil_component(VkFormat format);

      VkImageView view;
  private:
    uint32_t find_memory_type(uint32_t filter, VkMemoryPropertyFlags properties);


    VkDevice device;
    VkPhysicalDevice physical_device;

    VkImage image;
    VkDeviceMemory memory;
    uint32_t width;
    uint32_t height;
    VkFormat format;
    VkImageAspectFlags aspect;


    
};
