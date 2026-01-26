#include "Image.hpp"

#include <stdexcept>

Image::Image(
  VkDevice device, 
  VkPhysicalDevice physical_device,
  uint32_t width, 
  uint32_t height, 
  VkFormat format, 
  VkImageUsageFlags usage, 
  VkImageAspectFlags aspect, 
  VkMemoryPropertyFlags property
) : device(device), physical_device(physical_device), width(width), height(height), format(format), aspect(aspect) {

  VkImageCreateInfo image_info{}; 
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.format = format;
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = 1;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.usage = usage;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;

  if (vkCreateImage(device, &image_info, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create image");
  }
 
  VkMemoryRequirements mem_requirements; 
  vkGetImageMemoryRequirements(device, image, &mem_requirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = mem_requirements.size;
  allocInfo.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, property);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
    throw std::runtime_error("Unable to allocate device memory");
  }

  vkBindImageMemory(device, image, memory, 0);

  VkImageViewCreateInfo image_view_info{};
  image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_info.image = image;
  image_view_info.format = format;
  image_view_info.subresourceRange.aspectMask = aspect;
  image_view_info.subresourceRange.baseMipLevel = 0;
  image_view_info.subresourceRange.levelCount = 1;
  image_view_info.subresourceRange.baseArrayLayer = 0;
  image_view_info.subresourceRange.layerCount = 1;

  if (vkCreateImageView(device, &image_view_info, nullptr, &view) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create image view");
  }
}

uint32_t Image::find_memory_type(uint32_t filter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i) {
    if ((filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

void Image::transition_image_layout(CommandPool& command_pool, VkImageLayout old_layout, VkImageLayout new_layout) {
  VkCommandBuffer command = command_pool.start_single_command();

  VkImageMemoryBarrier image_barrier{};
  image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; 
  image_barrier.image = image;
  image_barrier.oldLayout = old_layout;
  image_barrier.newLayout = new_layout;
  image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_barrier.subresourceRange.aspectMask = aspect;
  image_barrier.subresourceRange.baseMipLevel = 0;
  image_barrier.subresourceRange.levelCount = 1;
  image_barrier.subresourceRange.baseArrayLayer = 0;
  image_barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags src_stage_mask;
  VkPipelineStageFlags dst_stage_mask;

  if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (has_stencil_component(format)) {
      image_barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    image_barrier.srcAccessMask = 0;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    
    src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    image_barrier.srcAccessMask = 0;
    image_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }

  vkCmdPipelineBarrier(
    command, 
    src_stage_mask, dst_stage_mask, 
    0,
    0, nullptr,
    0, nullptr,
    1, &image_barrier
  );

  command_pool.end_single_command(command);
}

void Image::copy_from_buffer(CommandPool& command_pool, Buffer& buffer) {
  VkCommandBuffer command = command_pool.start_single_command();
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = aspect;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {
    width,
    height,
    1
  };

  vkCmdCopyBufferToImage(command, buffer.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  command_pool.end_single_command(command);
}

bool Image::has_stencil_component(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

