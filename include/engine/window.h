#pragma once

#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

namespace Engine {

class Window {
public:
    bool m_frame_buffer_resized = false;

    void init_window();
    GLFWwindow* get_window();
    void cleanup();
    static void framebuffer_resize_callback(GLFWwindow *window, int width, int height);


private:
    GLFWwindow* m_window;
};

}