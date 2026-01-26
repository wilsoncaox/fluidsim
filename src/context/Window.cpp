
#include "Window.hpp"
#include <GLFW/glfw3.h>

#include <cmath>

Window::Window() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow(800, 600, "3D Physics Renderer", nullptr, nullptr);

  if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwSetKeyCallback(window, key_callback);

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);

};

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
  auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  app->height = height;
  app->width = width;


  app->set_origin_position({
    std::floor(width / 2.0),
    std::floor(height / 2.0)
  });
  app->resized = true;
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  if (action == GLFW_PRESS) {
    app->add_key_press(key);
  } else if (action == GLFW_RELEASE) {
    app->remove_key_press(key);
  }

}

void Window::poll_events() {
  glfwPollEvents();
}

void Window::wait_events() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
}

void Window::create_surface(VkInstance instance, VkSurfaceKHR* surface) {
  glfwCreateWindowSurface(instance, window, nullptr, surface);
}

void Window::get_window_size(int* width, int* height) {
  glfwGetFramebufferSize(window, width, height);
}

bool Window::should_window_close() {
  return glfwWindowShouldClose(window);
}

double const Window::time() {
  return glfwGetTime();
}

void Window::reset_cursor_position() {
  glfwSetCursorPos(window, origin.x, origin.y);
}


void Window::set_origin_position(CursorPosition cursor) {
  origin = cursor; 
}

CursorPosition const Window::get_cursor_position() {
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  return {
    xpos,
    ypos
  };
}


void Window::set_title(std::string title) {
  glfwSetWindowTitle(window, title.c_str());
}

