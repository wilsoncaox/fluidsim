
#include "Renderpass.hpp"

#include <array>
#include <stdexcept>
#include <iostream>
#include <vulkan/vulkan_core.h>

Renderpass::Renderpass(
  VkDevice device, 
  VkPhysicalDevice physical_device, 
  VkFormat surface_format, 
  VkFormat depth_format
) : device(device), physical_device(physical_device) {
  VkAttachmentDescription color_attachment{};   
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  color_attachment.format = surface_format;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  VkAttachmentDescription depth_attachment{};
  depth_attachment.format = depth_format;
  depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; 

  VkAttachmentReference color_ref{};
  color_ref.attachment = 0;
  color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_ref{};
  depth_ref.attachment = 1;
  depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpassDescription{}; 
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.pColorAttachments = &color_ref;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pDepthStencilAttachment = &depth_ref;

  VkSubpassDependency subpassDependenency{};
  subpassDependenency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependenency.dstSubpass = 0;
  subpassDependenency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  subpassDependenency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpassDependenency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  subpassDependenency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};

  VkRenderPassCreateInfo renderpass_info{};
  renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderpass_info.pAttachments = attachments.data();
  renderpass_info.subpassCount = 1;
  renderpass_info.pSubpasses = &subpassDescription;
  renderpass_info.dependencyCount = 1;
  renderpass_info.pDependencies = &subpassDependenency;

  if (vkCreateRenderPass(device, &renderpass_info,nullptr, &renderpass) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create render pass");
  }
}

Renderpass::~Renderpass() {
  clean_resources();
  vkDestroyRenderPass(device, renderpass, nullptr);
}

void Renderpass::init_resources(Swapchain& swapchain, CommandPool& command_pool) {
  create_depth_resource(swapchain, command_pool);
  create_framebuffers(swapchain);
}

void Renderpass::clean_resources() {
    depth_image.reset();
    for (auto framebuffer : framebuffers) {
      vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

}

void Renderpass::create_depth_resource(Swapchain& swapchain, CommandPool& command_pool) {
  depth_image = std::make_unique<Image>(
    device,
    physical_device,
    swapchain.extent.width,
    swapchain.extent.height,
    swapchain.find_depth_format(),
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    VK_IMAGE_ASPECT_DEPTH_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );
  
  depth_image->transition_image_layout(command_pool, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Renderpass::create_framebuffers(Swapchain& swapchain) {
  auto views = swapchain.views;

  framebuffers.resize(views.size());
  
  for (size_t i = 0; i < views.size(); ++i) {
    std::array<VkImageView, 2> attachments = {
      views[i],
      depth_image->view
    };
    VkFramebufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    buffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    buffer_info.pAttachments = attachments.data();
    buffer_info.height = swapchain.extent.height;
    buffer_info.width = swapchain.extent.width;
    buffer_info.layers = 1;
    buffer_info.renderPass = renderpass;

    if (vkCreateFramebuffer(device, &buffer_info, nullptr, &framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Unable to create frame buffer");
    }
  }
}

void Renderpass::begin_renderpass(Swapchain& swapchain, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  VkRenderPassBeginInfo begin_info{};
  begin_info.sType= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.renderPass = renderpass;
  begin_info.framebuffer = framebuffers[imageIndex];

  std::array<VkClearValue, 2> clear_values{};
  clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clear_values[1].depthStencil = {1.0f, 0};

  begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
  begin_info.pClearValues = clear_values.data();

  begin_info.renderArea = {
    {0, 0},
    swapchain.extent
  };

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float) swapchain.extent.width;
  viewport.height = (float) swapchain.extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapchain.extent;

  vkCmdBeginRenderPass(commandBuffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderpass::end_renderpass(VkCommandBuffer command_buffer) {
  vkCmdEndRenderPass(command_buffer);

}
