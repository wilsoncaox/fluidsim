
#include "Swapchain.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <iostream>

Swapchain::Swapchain(
  VulkanContext& context
) : device(context.device), physical_device(context.physical_device), surface(context.surface), queue(context.queue), queue_index(context.queue_index) {}

Swapchain::~Swapchain() {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device, finished_images[i], nullptr);
    vkDestroySemaphore(device, available_images[i], nullptr);
    vkDestroyFence(device, in_flight_fences[i], nullptr);
  }

  cleanup();

}

void Swapchain::create(Window& window) {
  create_swapchain(window);
  create_image_views();
  create_sync_objects();
}

void Swapchain::cleanup() {
  for (auto view : views) {
    vkDestroyImageView(device, view, nullptr);
  }

  vkDestroySwapchainKHR(device, swapchain, nullptr);
}


void Swapchain::create_swapchain(Window& window) {
  choose_present_mode();
  choose_surface_format();
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);  

  extent = choose_swap_extent(capabilities, window);

  uint32_t image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
    image_count = capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapchain_info{}; 
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = surface;
  swapchain_info.minImageCount = image_count;
  swapchain_info.imageFormat = format.format;
  swapchain_info.imageColorSpace = format.colorSpace;
  swapchain_info.imageExtent = extent;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  const uint32_t queue_value = queue_index.index.value();
  swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_info.queueFamilyIndexCount = 1;
  swapchain_info.pQueueFamilyIndices = &queue_value;

  swapchain_info.preTransform = capabilities.currentTransform;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode = mode;
  swapchain_info.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &swapchain) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create swap chain");
  }

  vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
  images.resize(image_count);
  vkGetSwapchainImagesKHR(device, swapchain, &image_count, images.data());

}

void Swapchain::create_image_views() {
  views.resize(images.size());

  for (uint32_t i = 0; i < views.size(); i++) {
    VkImageViewCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_info.image = images[i];
    image_info.format = format.format;
    image_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_info.subresourceRange.baseMipLevel = 0;
    image_info.subresourceRange.levelCount = 1;
    image_info.subresourceRange.baseArrayLayer = 0;
    image_info.subresourceRange.layerCount = 1;
    if (vkCreateImageView(device, &image_info, nullptr, &views[i]) != VK_SUCCESS) {
      throw std::runtime_error("Unable to create image view");
    }
  }
}

void Swapchain::create_sync_objects() {
  available_images.resize(MAX_FRAMES_IN_FLIGHT);  
  finished_images.resize(MAX_FRAMES_IN_FLIGHT); 
  in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device, &semaphore_info, nullptr, &available_images[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphore_info, nullptr, &finished_images[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
  }
}

void Swapchain::choose_present_mode() {
  uint32_t present_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_count, nullptr);
  std::vector<VkPresentModeKHR> presents(present_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_count, presents.data());

  // for (const auto& mode : presents) {
  //   if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
  //     this->mode = mode;
  //     return;
  //   }
  // }

  this->mode = VK_PRESENT_MODE_FIFO_KHR;
}


void Swapchain::choose_surface_format() {
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(format_count);

  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());

  for (const auto& format : formats) {

    if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      this->format = format;
      return;
    }
  }

  this->format = formats[0];
}

VkExtent2D Swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, Window& window) {
  if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
    int width, height;
    window.get_window_size(&width, &height);

    VkExtent2D extent {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height)
    };

    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
  } else {
    return capabilities.currentExtent;
  }
}


VkResult Swapchain::acquire_image(uint32_t* index, uint32_t current_frame) {
  vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
  auto result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, available_images[current_frame], VK_NULL_HANDLE, index); 
  return result;
}

VkResult Swapchain::submit_command(VkCommandBuffer command_buffer, uint32_t current_frame, uint32_t* image_index) {
  VkSemaphore available_render_image[] = {available_images[current_frame]};
  VkSemaphore finished_render_image[] = {finished_images[current_frame]};

  VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = available_render_image;
  submitInfo.pWaitDstStageMask = wait_stages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command_buffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = finished_render_image;

  vkResetFences(device, 1, &in_flight_fences[current_frame]);
  if (vkQueueSubmit(queue, 1, &submitInfo, in_flight_fences[current_frame]) != VK_SUCCESS) {
    throw std::runtime_error("Unable to submit command to queue");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = finished_render_image;

  VkSwapchainKHR swapChains[] = {swapchain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = image_index;

  auto result = vkQueuePresentKHR(queue, &presentInfo);

  return result;

}

VkFormat Swapchain::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

VkFormat Swapchain::find_depth_format() {
  return find_supported_format(
    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}


void Swapchain::reset_fence(uint32_t current_frame) {
  vkResetFences(device, 1, &in_flight_fences[current_frame]);
}
