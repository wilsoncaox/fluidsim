
#include "GraphicsPipeline.hpp"

#include "../resource/Vertex.hpp"
#include "Shader.hpp"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

GraphicsPipeline::GraphicsPipeline(
  VkDevice device, 
  std::string vertex, 
  std::string fragment
) : Pipeline(device), vertex_shader_path(vertex), fragment_shader_path(fragment) {};

GraphicsPipeline::~GraphicsPipeline() {
  if (pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(device, pipeline, nullptr);
  }

  if (layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device, layout, nullptr);
  }
}

void GraphicsPipeline::create(VkRenderPass renderpass, std::vector<VkDescriptorSetLayout>& descriptor_layouts) {
  VkPipelineVertexInputStateCreateInfo vertexInputStage{};
  vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  auto attributeDescriptions = Vertex::getAttributeDescription();
  auto bindingDescriptions = Vertex::getBindingDescription();

  vertexInputStage.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputStage.pVertexAttributeDescriptions = attributeDescriptions.data();
  vertexInputStage.vertexBindingDescriptionCount = 1;
  vertexInputStage.pVertexBindingDescriptions = &bindingDescriptions;

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStage{};
  inputAssemblyStage.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyStage.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyStage.primitiveRestartEnable = VK_FALSE;

  Shader vertexShader(device, vertex_shader_path, VK_SHADER_STAGE_VERTEX_BIT);   
  Shader fragShader(device, fragment_shader_path, VK_SHADER_STAGE_FRAGMENT_BIT);   
    
  std::array<VkPipelineShaderStageCreateInfo, 2> shaderInfo { vertexShader.getShaderInfo(), fragShader.getShaderInfo() };

  VkPipelineViewportStateCreateInfo viewportInfo{}; 
  viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportInfo.viewportCount = 1;
  viewportInfo.scissorCount = 1;
  
  VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
  rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
  rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationInfo.depthClampEnable = VK_FALSE; 
  rasterizationInfo.depthBiasEnable = VK_FALSE;
  rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizationInfo.lineWidth = 1.0f;
  rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

  VkPipelineMultisampleStateCreateInfo sampleInfo{};
  sampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  sampleInfo.sampleShadingEnable = VK_FALSE;
  sampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineDepthStencilStateCreateInfo depthInfo{};
  depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthInfo.depthTestEnable = VK_TRUE;
  depthInfo.depthWriteEnable = VK_TRUE;
  depthInfo.depthBoundsTestEnable = VK_FALSE;
  depthInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  depthInfo.stencilTestEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; 
  colorBlendAttachment.blendEnable = VK_TRUE; 
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; 
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; 
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; 
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; 
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; 
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

  VkPipelineColorBlendStateCreateInfo colorInfo{};
  colorInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorInfo.logicOpEnable = VK_FALSE;
  colorInfo.logicOp = VK_LOGIC_OP_COPY;
  colorInfo.attachmentCount = 1;
  colorInfo.pAttachments = &colorBlendAttachment;
  colorInfo.blendConstants[0] = 0.0f;
  colorInfo.blendConstants[1] = 0.0f;
  colorInfo.blendConstants[2] = 0.0f;
  colorInfo.blendConstants[3] = 0.0f;
  
  std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  
  VkPipelineDynamicStateCreateInfo dynamicInfo{};
  dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicInfo.pDynamicStates = dynamicStates.data();

  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size   = sizeof(glm::mat4);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptor_layouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptor_layouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create pipeline layout");
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.layout = layout;
  pipelineInfo.pVertexInputState = &vertexInputStage;
  pipelineInfo.pInputAssemblyState = &inputAssemblyStage;
  pipelineInfo.pViewportState = &viewportInfo;
  pipelineInfo.pStages = shaderInfo.data();
  pipelineInfo.pRasterizationState = &rasterizationInfo;
  pipelineInfo.pDepthStencilState = &depthInfo;
  pipelineInfo.pColorBlendState = &colorInfo;
  pipelineInfo.pMultisampleState = &sampleInfo;
  pipelineInfo.pDynamicState = &dynamicInfo;
  pipelineInfo.stageCount = 2;
  pipelineInfo.renderPass = renderpass;
  pipelineInfo.subpass = 0;

  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create graphpics pipeline");
  }
}

void GraphicsPipeline::bind_pipeline(VkCommandBuffer command_buffer) {
  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}
