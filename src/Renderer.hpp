
#pragma once

#include "resource/Mesh.hpp"
#include "scene/Scene.hpp"
#include "renderpass/Renderpass.hpp"
#include "pipeline/GraphicsPipeline.hpp"
#include "descriptors/DescriptorHandler.hpp"

#include "scene/pipelines/BoundaryPipeline.hpp"

#include <memory>

class Renderer {

  public:
    Renderer() = default;

    void init(VulkanContext& context, Window& window);
    void build_resources(VulkanContext& context, Scene& scene);
  
    void draw(VulkanContext& context, Window& window, Scene& scene, uint32_t current_frame);
    
  private:
    void recreate_frame(VulkanContext& context, Window& window); 

    std::unique_ptr<GraphicsPipeline> graphics_pipeline;
    std::unique_ptr<BoundaryPipeline> boundary_pipeline;

    std::vector<VkCommandBuffer> commandbuffers;


    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<Renderpass> renderpass;

    std::vector<Mesh> meshes;


    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
};
