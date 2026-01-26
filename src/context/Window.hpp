#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <unordered_set>
#include <string>

struct CursorPosition {
  double x;
  double y;
};

class Window {

  public:
    Window(); 
    ~Window();

    void poll_events();
    void wait_events();

    void get_window_size(int* width, int* height);  
    bool should_window_close();
    void create_surface(VkInstance instance, VkSurfaceKHR* surface);
    double const time();

    void reset_cursor_position();
    void set_origin_position(CursorPosition cursor);
    CursorPosition const get_origin_position() const { return origin; };
    CursorPosition const get_cursor_position();

    void add_key_press(int key) { key_pressed.insert(key); }
    void remove_key_press(int key) { key_pressed.erase(key); }
    bool pressed(int key) { return key_pressed.find(key) != key_pressed.end(); }

    static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    void set_title(std::string title);

    bool resized = false;

    int width;
    int height;

  private:
    GLFWwindow* window; 
    CursorPosition origin;
    std::unordered_set<int> key_pressed;


};
