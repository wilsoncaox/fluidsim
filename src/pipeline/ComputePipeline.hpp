#pragma once

#include "Pipeline.hpp"

#include <vector>
#include <string>

class ComputePipeline : public Pipeline {

  public:
    ComputePipeline(VkDevice device, std::string compute);
    ~ComputePipeline();

    void create(std::vector<VkDescriptorSetLayout>&& descriptor_set_layout = {}, std::vector<VkPushConstantRange>&& push_constants = {});
    void bind_pipeline(VkCommandBuffer command_buffer) override;

  private:

    std::string compute_shader_path;
};
