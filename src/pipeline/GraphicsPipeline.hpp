#pragma once

#include "Pipeline.hpp"

#include <string>
#include <vector>

class GraphicsPipeline : public Pipeline {
  
  public:
    GraphicsPipeline(VkDevice device,  std::string vertex, std::string fragment); 
    ~GraphicsPipeline();
    void create(VkRenderPass renderpass, std::vector<VkDescriptorSetLayout>&& descriptor_layouts);
    void bind_pipeline(VkCommandBuffer command_buffer) override;

    void add_set(VkDescriptorSet set);
    void add_layout(VkDescriptorSetLayout layout);

  private:
    std::vector<VkDescriptorSetLayout> layouts;
    std::vector<VkDescriptorSet> sets;

    std::string vertex_shader_path;
    std::string fragment_shader_path;

};
