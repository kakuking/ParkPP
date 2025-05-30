#pragma once
#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

#include <iostream>
#include <stdexcept>

#include <engine/window.h>
#include <engine/pipeline.h>

namespace Engine {

struct QueueFamilyIndices
{
    uint32_t graphics_family;
};

class Renderer {
public:
    void run();
private:
    void initialize();
    void loop();
    void cleanup();

    void create_instance();
    void create_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swapchains();
    void create_graphics_pipeline();

    Window m_window;
    vkb::Instance m_instance;
    vkb::PhysicalDevice m_physical_device;
    vkb::Device m_device;
    vkb::DispatchTable m_dispatch;
    VkQueue m_graphics_queue, m_present_queue;
    VkSurfaceKHR m_surface = nullptr;

    vkb::Swapchain m_swapchain;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
    Pipeline m_graphics_pipeline;
};

}
