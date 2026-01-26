
#include "ComputePipeline.hpp"
#include "Shader.hpp"

#include <stdexcept>

ComputePipeline::ComputePipeline(VkDevice device, std::string compute) : Pipeline(device), compute_shader_path(compute) {}

ComputePipeline::~ComputePipeline() {
  if (pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(device, pipeline, nullptr);
  }

  if (layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device, layout, nullptr);
  }
}


void ComputePipeline::create(std::vector<VkDescriptorSetLayout>&& descriptor_set_layouts, std::vector<VkPushConstantRange>&& push_constants) {
   VkPipelineLayoutCreateInfo layout_info{};
   layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
   layout_info.pSetLayouts = (descriptor_set_layouts.size() == 0) ? nullptr : descriptor_set_layouts.data();
   layout_info.pushConstantRangeCount = static_cast<uint32_t>(push_constants.size());
   layout_info.pPushConstantRanges = (push_constants.size() == 0) ? nullptr : push_constants.data();

  if (vkCreatePipelineLayout(device, &layout_info, nullptr, &layout) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create pipline layout");
  }

  Shader shader(device, compute_shader_path, VK_SHADER_STAGE_COMPUTE_BIT);

  VkComputePipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.stage = shader.getShaderInfo();
    pipeline_info.layout = layout;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = 0;

  if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create pipeline");
  }
}

void ComputePipeline::bind_pipeline(VkCommandBuffer commandbuffer) {
  vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}


