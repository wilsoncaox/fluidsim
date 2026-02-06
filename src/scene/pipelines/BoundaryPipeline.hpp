
#include "../../pipeline/Pipeline.hpp"

#include <vector>
#include <string>

class BoundaryPipeline : public Pipeline {

  public:
    BoundaryPipeline(
        VkDevice device, 
        std::string vertex, 
        std::string fragment
        );
    ~BoundaryPipeline();
    void bind_pipeline(VkCommandBuffer command_buffer) override;
     
    void create(VkRenderPass renderpass, std::vector<VkDescriptorSetLayout>&& descriptor_layouts);
    
  private:
    std::vector<VkDescriptorSetLayout> layouts;
    std::vector<VkDescriptorSet> sets;

    std::string vertex_shader_path;
    std::string fragment_shader_path;

};
