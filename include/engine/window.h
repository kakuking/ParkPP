#pragma once

#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

namespace Engine {

class Window {
public:
    void init_window();
    GLFWwindow* get_window();
    void cleanup();

private:
    GLFWwindow* m_window;
};

}