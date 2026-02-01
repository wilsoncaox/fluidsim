
#pragma once

#include "../buffer/HostBuffer.hpp"
#include "../context/Window.hpp"
#include "../descriptors/DescriptorBuilder.hpp"
#include "../pipeline/Pipeline.hpp"
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

struct CameraData {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Camera {
  public:
    Camera(VkDevice device, VkPhysicalDevice physical_device, DescriptorBuilder& builder);

    void bind_camera(VkCommandBuffer commandbuffer, Pipeline& pipeline, VkPipelineBindPoint bind_point);
    void update(Window& window, float delta_time);

    double yaw = -105;
    double pitch = -20;
    glm::vec3 position = {0, 20, 20};

    VkDescriptorSetLayout layout;

  private:

    VkDevice device;
    VkPhysicalDevice physical_device;
    uint32_t binding;

    std::unique_ptr<HostBuffer> buffer;
    VkDescriptorSet set;
};
