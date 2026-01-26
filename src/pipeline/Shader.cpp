
#include "Shader.hpp"
#include <fstream>
#include <stdexcept>


Shader::Shader(VkDevice device, const std::string& filename, VkShaderStageFlagBits stage) : device(device), stage(stage) {
  std::vector<char> file = readFile(filename);

  VkShaderModuleCreateInfo shader_info{};
  shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;   
  shader_info.codeSize = file.size();
  shader_info.pCode = reinterpret_cast<const uint32_t*>(file.data());

  if (vkCreateShaderModule(device, &shader_info, nullptr, &module) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create shader module");
  }
}

Shader::~Shader() {
  vkDestroyShaderModule(device, module, nullptr);  
}

std::vector<char> Shader::readFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file");
  }

  size_t filesize = (size_t) file.tellg();
  std::vector<char> buffer(filesize);

  file.seekg(0);
  file.read(buffer.data(), filesize);
  file.close();

  return buffer;
}

VkPipelineShaderStageCreateInfo Shader::getShaderInfo() {
  VkPipelineShaderStageCreateInfo shaderInfo {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .stage = stage,
    .module = module,
    .pName = "main",
    .pSpecializationInfo = nullptr
  };

  return shaderInfo;
}

