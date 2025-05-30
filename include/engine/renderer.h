#pragma once
#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

#include <iostream>
#include <stdexcept>

#include <engine/window.h>
#include <engine/pipeline.h>

const int MAX_FRAMES_IN_FLIGHT = 2;

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
    void create_framebuffers();
    void create_command_pool();
    void create_command_buffers();
    void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);

    void create_sync_objects();
    void draw_frame();

    Window m_window;
    vkb::Instance m_instance;
    vkb::PhysicalDevice m_physical_device;
    vkb::Device m_device;
    vkb::DispatchTable m_dispatch;
    VkQueue m_graphics_queue, m_present_queue;
    uint32_t m_graphics_queue_idx, m_present_queue_idx;
    VkSurfaceKHR m_surface = nullptr;

    vkb::Swapchain m_swapchain;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
    std::vector<VkFramebuffer> m_swapchain_framebuffers;

    Pipeline m_graphics_pipeline;
    VkCommandPool m_command_pool;
    std::vector<VkCommandBuffer> m_command_buffers;

    std::vector<VkSemaphore> m_image_available_semaphores, m_render_finished_semaphores;
    std::vector<VkFence> m_in_flight_fences;

    uint32_t m_current_frame = 0;
};

}
