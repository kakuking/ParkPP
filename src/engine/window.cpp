#include <engine/window.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <iostream>

namespace Engine {

void Window::init_window() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
        throw std::runtime_error("Failed to initialize SDL!");

    if (SDL_Vulkan_LoadLibrary(nullptr) < 0) {
        throw std::runtime_error("Failed to load vulkan loader");
        SDL_Quit();
    }

    m_window = SDL_CreateWindow("Pogger Park",
                                WIDTH, HEIGHT,
                                SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    if (!m_window) {
        throw std::runtime_error("Failed to create window!");
        SDL_Quit();
    }

    std::cout << "Created window\n";
}

VkResult Window::create_window_surface(VkInstance instance, VkSurfaceKHR &surface) {
    std::cout << "Creating surface\n";
    if (!SDL_Vulkan_CreateSurface(m_window, instance, nullptr, &surface)) {
        std::cerr << "Failed to create Vulkan surface: " << SDL_GetError() << std::endl;
        return VK_ERROR_SURFACE_LOST_KHR; // fallback error code
    }
    return VK_SUCCESS;
}

SDL_Window* Window::get_window() {
    return m_window;
}

void Window::cleanup() {
    if(m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}

std::vector<const char*> Window::get_required_instance_extensions() const {
    Uint32 count = 0;

    const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&count);
    if (!extensions) {
        throw std::runtime_error(SDL_GetError());
    }
    return std::vector<const char*>(extensions, extensions + count);
}

void Window::poll_events() {
    SDL_Event event;

    while(SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            if (event.window.windowID == SDL_GetWindowID(m_window)) {
                m_frame_buffer_resized = true;
                current_width = event.window.data1;
                current_height = event.window.data2;
            }
        } else if (event.type == SDL_EVENT_QUIT)
            m_should_close = true;
    }
}

void Window::wait_events() {
    SDL_Event event;
    SDL_WaitEvent(&event);
    poll_events();
}

void Window::get_framebuffer_size(int &width, int &height) {
    int drawable_w, drawable_h;
    SDL_GetWindowSizeInPixels(m_window, &drawable_w, &drawable_h);
    width = drawable_w;
    height = drawable_h;
}

bool Window::window_should_close() {
    return m_should_close;
}

}