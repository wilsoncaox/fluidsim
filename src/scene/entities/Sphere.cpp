
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "Sphere.hpp"

Sphere::Sphere() {
  load_mesh();
}

void Sphere::load_mesh() {
  float radius = 0.1f;
  uint32_t stacks = 32;
  uint32_t slices = 32;

  for (uint32_t i = 0; i <= stacks; i++) {
    float v = (float)i / stacks;
    float phi = v * glm::pi<float>();

    for (uint32_t j = 0; j <= slices; j++) {
      float u = (float)j / slices;
      float theta = u * glm::two_pi<float>();

      glm::vec3 pos{
        radius * sin(phi) * cos(theta),
        radius * cos(phi),
        radius * sin(phi) * sin(theta)
      };

      glm::vec3 normal = glm::normalize(pos);

      vertices.push_back({
        pos,
        {0.74f, 0.44f, 0.47f},
        normal,
        {u, v}
      });
    }
  } 

  for (uint32_t i = 0; i < stacks; i++) {
    for (uint32_t j = 0; j < slices; j++) {
      uint32_t first  = i * (slices + 1) + j;
      uint32_t second = first + slices + 1;

      indices.push_back(first);
      indices.push_back(second);
      indices.push_back(first + 1);

      indices.push_back(second);
      indices.push_back(second + 1);
      indices.push_back(first + 1);
    }
  }
}
