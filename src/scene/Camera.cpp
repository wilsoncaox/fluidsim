
#include "Camera.hpp"
#include <iostream>

Camera::Camera(
  VkDevice device, 
  VkPhysicalDevice physical_device,
  uint32_t binding
) : device(device), physical_device(physical_device), binding(binding) {
 
  buffer = std::make_unique<HostBuffer>(device, physical_device, sizeof(CameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  resource = std::make_unique<UniformBuffer>(device, 0, VK_SHADER_STAGE_VERTEX_BIT); 

  resource->bind(buffer.get());
}

void Camera::update(Window& window, float delta_time) {
  CursorPosition current = window.get_cursor_position();
  CursorPosition center  = window.get_origin_position();

  float dx = current.x - center.x;
  float dy = center.y - current.y;

  float sensitivity = 0.05f;

  yaw += dx * sensitivity;
  pitch += dy * sensitivity;

  pitch = glm::clamp(pitch, -89.0, 89.0);

  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  front = glm::normalize(front);

  glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
  glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
  glm::vec3 up    = glm::normalize(glm::cross(right, front));

  float speed = 10.0f;

  if (window.pressed(GLFW_KEY_W)) position += front * speed * delta_time;
  if (window.pressed(GLFW_KEY_S)) position -= front * speed * delta_time;
  if (window.pressed(GLFW_KEY_D)) position += right * speed * delta_time;
  if (window.pressed(GLFW_KEY_A)) position -= right * speed * delta_time;
  if (window.pressed(GLFW_KEY_SPACE)) position += up * speed * delta_time;
  if (window.pressed(GLFW_KEY_LEFT_SHIFT)) position -= up * speed * delta_time;

  CameraData data{};  

  data.model = glm::mat4(1.0f); 
  data.view = glm::lookAt(position, front + position , up);

  int width, height;
  window.get_window_size(&width, &height);
  data.proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 500.0f);
  data.proj[1][1] *= -1;

  buffer->fillData(&data, sizeof(data));
  
  window.reset_cursor_position();
}
