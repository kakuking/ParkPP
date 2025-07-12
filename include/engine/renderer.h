#pragma once
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>

#include <engine/window.h>

#include <engine/image.h>

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace Engine {
class Pipeline;
class Texture;

class Renderer {
public:

    // Call this first
    void initialize_vulkan();
    // Create buffers & pipeline then call this next
    // [binding][frame]
    void initialize(
        std::vector<Pipeline*> pipelines,
        const std::vector<std::vector<uint32_t>>& uniform_buffer_indices,
        const std::vector<std::vector<uint32_t>>& uniform_buffer_sizes
    );

    // Return a new command buffer to be filled in
    bool begin_frame(int &current_frame, uint32_t &image_index, VkCommandBuffer &out_buffer);
    // Draw to the swapchain and perform sync
    void end_frame();
    
    void cleanup();

    // Returns the index of the created buffer
    size_t create_index_buffer(VkDeviceSize buffer_size);
    size_t create_vertex_buffer(VkDeviceSize buffer_size);
    size_t create_uniform_buffer(VkDeviceSize buffer_size);
    size_t create_buffer(VkDeviceSize buffer_size, uint32_t usage, uint32_t memory_props, bool per_frame=false);
    void update_buffer(size_t buffer_idx, void* src_data, size_t src_data_size);

    void add_descriptor_set_layout_binding(VkDescriptorSetLayoutBinding binding, VkDescriptorBindingFlags binding_flag);
    
    VkDescriptorSetLayout get_descriptor_set_layout() { return m_descriptor_set_layout; }
    
    vkb::DispatchTable m_dispatch;

    VkCommandBuffer begin_single_time_command();
    void end_single_time_command(VkCommandBuffer command_buffer);

    void copy_buffer_to_image(size_t buffer_idx, VkImage &image, uint32_t width, uint32_t height, uint32_t layer=0);

    void add_texture(std::string filename, uint32_t binding);
    void add_texture_array(std::vector<std::string> filename, uint32_t width, uint32_t height, uint32_t layer_count, uint32_t binding);

    bool window_should_close();
    VkRenderPass get_render_pass() { return m_render_pass; }
    VkFramebuffer get_framebuffer(int image_index) { return m_swapchain_framebuffers[image_index]; }
    VkExtent2D get_swapchain_extent() { return m_swapchain.extent; }
    VkBuffer get_buffer(size_t idx) { return m_buffers[idx]; }
    VkDescriptorSet get_descriptor_set(int idx) { return m_descriptor_sets[idx]; }
    vkb::Swapchain get_swapchain() { return m_swapchain; }
    uint32_t find_memory_type(uint32_t filter, VkMemoryPropertyFlags props);
    VkPhysicalDeviceProperties get_physical_device_properties() { return m_physical_device_properties; }
    VkFormat find_depth_format();
    VkSampleCountFlagBits get_msaa_sample_count() { return m_msaa_samples; }

private:

    void create_instance();
    void create_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swapchains();

    // Must set a render pass before calling this
    void create_framebuffers();
    void create_command_pool();
    void create_command_buffer();

    void create_sync_objects();

    void recreate_swap_chain();
    void cleanup_swapchain();

    void create_descriptor_pool();
    // [binding][frame]
    void create_descriptor_sets(
        const std::vector<std::vector<uint32_t>>& uniform_buffer_indices,
        const std::vector<std::vector<uint32_t>>& uniform_buffer_sizes
    );
    void create_descriptor_set_layout();

    void destroy_buffer(int buffer_idx);
    void destroy_pipeline(int pipeline_idx);

    void create_depth_resources();
    void create_color_resources();
    VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    bool has_stencil_component(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
    VkSampleCountFlagBits get_max_usable_sample_count();

    
    // Vulkan context
    vkb::Instance m_instance;
    vkb::InstanceDispatchTable m_instance_dispatch;
    vkb::PhysicalDevice m_physical_device;
    VkPhysicalDeviceProperties m_physical_device_properties;
    vkb::Device m_device;

    // Queues
    VkQueue m_graphics_queue, m_present_queue;
    uint32_t m_graphics_queue_idx, m_present_queue_idx;
    
    // Surfaces and swapchains
    VkSurfaceKHR m_surface;
    vkb::Swapchain m_swapchain;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
    std::vector<VkFramebuffer> m_swapchain_framebuffers;
    VkRenderPass m_render_pass = VK_NULL_HANDLE;

    // Command objects
    VkCommandPool m_command_pool;
    std::vector<VkCommandBuffer> m_command_buffers;

    // Descriptor objects
    VkDescriptorPool m_descriptor_pool;
    std::vector<VkDescriptorSetLayoutBinding> m_descriptor_bindings;
    std::vector<VkDescriptorBindingFlags> m_descriptor_binding_flags;
    VkDescriptorSetLayout m_descriptor_set_layout;
    std::vector<VkDescriptorSet> m_descriptor_sets;
    uint32_t num_buffer_descriptor_sets = 0, num_image_descriptor_sets = 0;

    // buffers and buffer_memories
    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_buffer_memories;

    std::vector<Pipeline*> m_pipelines;

    std::vector<TextureImage> m_textures;
    std::vector<TextureImageArray> m_texture_arrays;

    // Window object
    Window m_window;

    // Sync objects
    std::vector<VkSemaphore> m_image_available_semaphores, m_render_finished_semaphores;
    std::vector<VkFence> m_in_flight_fences;

    uint32_t m_current_frame = 0;
    uint32_t m_current_image_index = 0;

    // depth resources
    DepthImage m_depth;

    // for msaa
    VkSampleCountFlagBits m_msaa_samples = VK_SAMPLE_COUNT_1_BIT;
    ColorImage m_color_image;
};
}