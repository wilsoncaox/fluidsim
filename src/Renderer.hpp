
#pragma once

#include "pipeline/ComputePipeline.hpp"
#include "resource/Mesh.hpp"
#include "scene/Scene.hpp"
#include "renderpass/Renderpass.hpp"
#include "pipeline/GraphicsPipeline.hpp"
#include "descriptors/DescriptorHandler.hpp"

#include <memory>

class Renderer {

  public:
    Renderer() = default;

    void init(VulkanContext& context, Window& window);
    void build_meshes(VulkanContext& context, Scene& scene);
    void build_resources(VulkanContext& context, Scene& scene);
    VkPipelineLayout add_graphics_pipeline(VulkanContext& context, std::string vertex, std::string fragment, std::vector<VkDescriptorSetLayout> graphic_layout);
    VkPipelineLayout add_compute_pipeline(VulkanContext& context, std::string compute, std::vector<VkDescriptorSetLayout> compute_layout);
  
    void draw(Window& window, Scene& scene, DescriptorHandler& handler, uint32_t current_frame);
    
    std::vector<std::unique_ptr<GraphicsPipeline>> graphics_pipelines;


    std::unique_ptr<ComputePipeline> scan; 
    std::unique_ptr<ComputePipeline> histogram;
    std::unique_ptr<ComputePipeline> sort;
    std::unique_ptr<ComputePipeline> particle_sim_pipeline;
    std::unique_ptr<ComputePipeline> pre_calc_position_pipeline;
    std::unique_ptr<ComputePipeline> pre_calc_density_pipeline;

  private:
    void recreate_frame(Window& window); 

    std::vector<VkCommandBuffer> commandbuffers;

    std::unique_ptr<CommandPool> commandpool;

    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<Renderpass> renderpass;

    std::vector<Mesh> meshes;


    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
};
