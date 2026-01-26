#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <array>

struct Vertex {
  glm::vec3 position;   
  glm::vec3 color;
  glm::vec3 normal;
  glm::vec2 uv;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription description{};
    description.binding = 0;
    description.stride = sizeof(Vertex);
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return description;
  };

  static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescription() {
    std::array<VkVertexInputAttributeDescription, 4> description{};
    description[0].binding = 0;
    description[0].location = 0;
    description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    description[0].offset = offsetof(Vertex, position);

    description[1].binding = 0;
    description[1].location = 1;
    description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    description[1].offset = offsetof(Vertex, color);

    description[2].binding = 0;
    description[2].location = 2;
    description[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    description[2].offset = offsetof(Vertex, normal);

    description[3].binding = 0;
    description[3].location = 3;
    description[3].format = VK_FORMAT_R32G32_SFLOAT;
    description[3].offset = offsetof(Vertex, uv);

    return description;
  };
};

