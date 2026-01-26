#pragma once

#include "../buffer/Buffer.hpp"
#include "../scene/Entity.hpp"

#include <memory>

class Mesh {

  public:
    Mesh(VkDevice device, VkPhysicalDevice physical_device, Entity& entity, CommandPool& command_pool, uint32_t instances);

    void bind(VkCommandBuffer commandbuffer);
    void draw(VkCommandBuffer commandbuffer);
    uint32_t instances;

  private:
    VkDevice device;
    VkPhysicalDevice physical_device;
    uint32_t index_count;

    void create_vertex_buffer(Entity& entity, CommandPool& command_pool);
    void create_index_buffer(Entity& entity, CommandPool& command_pool);
    std::unique_ptr<Buffer> vertex_buffer;
    std::unique_ptr<Buffer> index_buffer;
};
