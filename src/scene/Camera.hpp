
#pragma once

#include "../buffer/resource/UniformBuffer.hpp"
#include "../buffer/HostBuffer.hpp"
#include "../context/Window.hpp"

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
    Camera(VkDevice device, VkPhysicalDevice physical_device, uint32_t binding);


    Descriptor* get_resource() {return resource.get(); };
    void update(Window& window, float delta_time);

    double yaw = -105;
    double pitch = -20;

    glm::vec3 position = {0, 20, 20};

  private:
   
    VkDevice device;
    VkPhysicalDevice physical_device;
    uint32_t binding;

    VkDescriptorBufferInfo buffer_info;

    std::unique_ptr<HostBuffer> buffer;
    std::unique_ptr<UniformBuffer> resource;
};
