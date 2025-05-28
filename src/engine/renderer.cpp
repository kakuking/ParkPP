#include <engine/renderer.h>
#include <fmt/format.h>

namespace Engine {

    void Renderer::run() {
        m_window.init_window();

        initialize();
        loop();
        cleanup();
    }

    void Renderer::initialize() {
        std::cout << "Init\n";
        create_instance();
        create_surface();
        pick_physical_device();
        create_logical_device();
        create_swapchains();
    }

    void Renderer::loop() {
        while(!glfwWindowShouldClose(m_window.get_window()))
            glfwPollEvents();
    }

    void Renderer::cleanup() {
        std::cout << "Cleaning up\n";

        m_swapchain.destroy_image_views(m_swapchain_image_views);
        vkb::destroy_swapchain(m_swapchain);
        vkb::destroy_device(m_device);
        vkb::destroy_surface(m_instance, m_surface);
        vkb::destroy_instance(m_instance);
        m_window.cleanup();
    }

    void Renderer::create_instance() {
        std::cout << "Creating instance\n";

        // Get extensions
        uint32_t glfw_extn_count = 0;
        const char** glfw_extns;
        std::vector<const char *> extensions;
        glfw_extns = glfwGetRequiredInstanceExtensions(&glfw_extn_count);

        for(uint32_t i = 0; i < glfw_extn_count; i++)
            extensions.push_back(glfw_extns[i]);

        auto system_info_ret = vkb::SystemInfo::get_system_info(0);
        if (!system_info_ret)
            throw std::runtime_error("Could not get system info");
        auto system_info = system_info_ret.value();

        if(!system_info.validation_layers_available) {
            std::cout << "No validation layers available!\n";
        }

        vkb::InstanceBuilder builder;
        vkb::Result<vkb::Instance> instance_ret = builder.set_app_name("Pogger Park")
                                    .set_app_version(1, 0, 0)
                                    .set_engine_name("No Engine")
                                    .set_engine_version(1, 0, 0)
                                    .enable_extensions(extensions)
                                    .request_validation_layers()
                                    .use_default_debug_messenger()
                                    .build();

        if(!instance_ret)
            throw std::runtime_error("Could not build Vulkan instance");

        m_instance = instance_ret.value();
    }

    void Renderer::create_surface() {
        VkResult res = glfwCreateWindowSurface(m_instance, m_window.get_window(), nullptr, &m_surface);
        if(res != VK_SUCCESS)
            throw std::runtime_error("Failed to create a window surface!");
    }

    void Renderer::pick_physical_device() {
        vkb::PhysicalDeviceSelector selector(m_instance);

        // Here you would ask for requirements normally
        auto selector_ret = selector.require_present().set_surface(m_surface).select();

        if(!selector_ret) {
            std::cout << "Error: " << selector_ret.error() << "\n\n";
            throw std::runtime_error("Could not select a physical device!");
        }
        
        m_physical_device = selector_ret.value();
    }

    // Creates device and gets graphics+present queue
    void Renderer::create_logical_device() {
        vkb::DeviceBuilder builder(m_physical_device);
        auto builder_ret = builder.build();
        if(!builder_ret)
            throw std::runtime_error("Could not create logical device");
        
        m_device = builder_ret.value();

        auto graphics_queue_ret = m_device.get_queue(vkb::QueueType::graphics);
        if(!graphics_queue_ret)
            throw std::runtime_error("Could not find a graphics queue");
            
        m_graphics_queue = graphics_queue_ret.value();

        auto present_queue_ret = m_device.get_queue(vkb::QueueType::present);
        if(!present_queue_ret)
            throw std::runtime_error("Could not find a present queue");

        m_present_queue = present_queue_ret.value();
    }

    void Renderer::create_swapchains() {
        vkb::SwapchainBuilder builder(m_device);

        auto swapchain_ret = builder.build();
        if(!swapchain_ret) {
            throw std::runtime_error("Could not build swapchains!");
        }
        
        m_swapchain = swapchain_ret.value();
        auto swapchain_images_ret = m_swapchain.get_images();
        if(!swapchain_images_ret)
            throw std::runtime_error("Could not get swapchain images");

        m_swapchain_images = swapchain_images_ret.value();

        auto swapchain_image_views_ret = m_swapchain.get_image_views();
        if(!swapchain_image_views_ret)
            throw std::runtime_error("Could not get swapchain image views");

        m_swapchain_image_views = swapchain_image_views_ret.value();

        m_swapchain_image_format = m_swapchain.image_format;
        m_swapchain_extent = m_swapchain.extent;
    }

    void Renderer::create_graphics_pipeline() {
        
    }
};