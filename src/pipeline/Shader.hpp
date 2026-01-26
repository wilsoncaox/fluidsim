#pragma once

#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

class Shader {
  
  public:
    Shader(VkDevice device, const std::string& filename, VkShaderStageFlagBits stage);
    ~Shader();  

    VkPipelineShaderStageCreateInfo getShaderInfo();

    VkShaderModule module;
  private:
    VkDevice device;
    VkShaderStageFlagBits stage;
    std::vector<char> readFile(const std::string& filename);
};

