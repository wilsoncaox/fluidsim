#pragma once

#include "../context/VulkanContext.hpp"

class Swapchain {

  public:
    Swapchain(VulkanContext& context); 
    ~Swapchain();

    void create(Window& window);
    void cleanup();

    VkResult acquire_image(uint32_t* index, uint32_t frame);
    void reset_fence(uint32_t frame);

    VkResult submit_command(VkCommandBuffer command_buffer, uint32_t frame, uint32_t* image_index);

    VkFormat find_depth_format();

    std::vector<VkImageView> views;
    VkExtent2D extent;
    VkSurfaceFormatKHR format;

    inline static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 4;

  private:

    void create_swapchain(Window& window);
    void create_image_views();

    void create_sync_objects();

    void choose_surface_format();
    void choose_present_mode();
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, Window& window);

    VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);


    VkDevice device;
    VkPhysicalDevice physical_device;
    VkQueue queue;
    QueueIndices queue_index;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;

    std::vector<VkImage> images;

    std::vector<VkSemaphore> available_images;
    std::vector<VkSemaphore> finished_images;
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> images_in_flight;;

    uint32_t image_index;

    VkPresentModeKHR mode;

};


