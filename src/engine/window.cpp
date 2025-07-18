#include <engine/window.h>

#include <iostream>

namespace Engine {

void Window::init_window() {
    glfwInit();

    // Not using OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Non-resizable window for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); 

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Pogger Park", nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_resize_callback);
}

void Window::framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    
    self->m_frame_buffer_resized = true;
    self->current_width = width;
    self->current_height = height;
}

GLFWwindow* Window::get_window() {
    return m_window;
}

void Window::cleanup() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

std::vector<const char*> Window::get_required_instance_extensions() const {
    uint32_t count = 0;
    const char** exts = glfwGetRequiredInstanceExtensions(&count);
    return std::vector<const char*>(exts, exts + count);
}

void Window::poll_events() {
    glfwPollEvents(); 
}

void Window::wait_events() {
    glfwWaitEvents(); 
}

VkResult Window::create_window_surface(VkInstance instance, VkSurfaceKHR &surface) {
    return glfwCreateWindowSurface(instance, m_window, nullptr, &surface); 
}

void Window::get_framebuffer_size(int &width, int &height) {
    glfwGetFramebufferSize(m_window, &width, &height); 
}

bool Window::window_should_close() {
    return glfwWindowShouldClose(m_window); 
}

}