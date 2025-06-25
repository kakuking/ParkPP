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
}

GLFWwindow* Window::get_window() {
    return m_window;
}

void Window::cleanup() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

}