#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

namespace Engine {

class Window {
public:
    bool m_frame_buffer_resized = false;
    int current_width = WIDTH, current_height = HEIGHT;

    void init_window();
    GLFWwindow* get_window();
    void cleanup();
    static void framebuffer_resize_callback(GLFWwindow *window, int width, int height);
    void poll_events();
    void wait_events();

    VkResult create_window_surface(VkInstance instance, VkSurfaceKHR &surface);
    void get_framebuffer_size(int &width, int &height);
    
    bool window_should_close();
    std::vector<const char*> get_required_instance_extensions() const;


private:
    GLFWwindow* m_window;
};

}