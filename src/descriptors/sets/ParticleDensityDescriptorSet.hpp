
#include "DescriptorSet.hpp"

#include <vector>
#include <string>

class ParticleDensityDescriptorSet : public DescriptorSet {

  public:

    ParticleDensityDescriptorSet(
      VkPipelineLayout layout, 
      VkPipelineBindPoint bind_point, 
      std::vector<VkBuffer> buffers, 
      std::vector<VkDescriptorSet> descriptor_sets, 
      uint32_t offset,
      uint32_t set_binding
    ); 

    void bind(VkCommandBuffer commandbuffer, uint32_t frame) override;
    VkBuffer get_buffer(uint32_t current_frame) override;

  private:
    

    VkPipelineLayout layout;
    VkPipelineBindPoint bind_point;
    std::vector<VkBuffer> buffers; 
    std::vector<VkDescriptorSet> descriptor_sets;
    uint32_t offset; 
    uint32_t set_binding;
};
