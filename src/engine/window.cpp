#include <engine/window.h>
#include <iostream>

namespace Engine {

void Window::init_window() {
    glfwInit();

    // Not using OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Non-resizable window for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); 

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Pogger Park", nullptr, nullptr);
    std::cout << "Created window!\n";
}

GLFWwindow* Window::get_window() {
    return m_window;
}

void Window::cleanup() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

}