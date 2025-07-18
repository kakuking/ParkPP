#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

namespace Engine {

class Window {
public:
    bool m_frame_buffer_resized = false;
    int current_width = WIDTH, current_height = HEIGHT;

    void init_window();
    SDL_Window* get_window();
    void cleanup();
    void poll_events();
    void wait_events();

    VkResult create_window_surface(VkInstance instance, VkSurfaceKHR &surface);
    void get_framebuffer_size(int &width, int &height);
    
    bool window_should_close();
    std::vector<const char*> get_required_instance_extensions() const;

private:
    SDL_Window* m_window = nullptr;

    bool m_should_close = false;
};

}