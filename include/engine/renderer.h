#pragma once
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>

#include <engine/window.h>
#include <engine/image.h>
#include <engine/pipeline.h>

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace Engine {
class Pipeline;
class Texture;
struct Model;

struct UniformBufferGroup {
    size_t m_base_index;
    size_t m_size;
    uint32_t m_binding;
};

struct Light {
    glm::mat4 mvp;
    VkImageView image_view;
    VkFramebuffer framebuffer;

    void cleanup(vkb::DispatchTable &dispatch_table) {
        dispatch_table.destroyImageView(image_view, nullptr);
        dispatch_table.destroyFramebuffer(framebuffer, nullptr);
    }
};

class Renderer {
public:
    // Call this first
    void initialize_vulkan();
    // Create buffers & pipeline then call this next
    // [binding][frame]
    void initialize(
        std::vector<Pipeline*> pipelines
    );

    // Return a new command buffer to be filled in
    bool begin_frame(int &current_frame, uint32_t &image_index, VkCommandBuffer &out_buffer);

    void begin_render_pass(VkCommandBuffer &command_buffer, uint32_t image_index);
    void bind_pipeline_and_descriptors(VkCommandBuffer &command_buffer, int pipeline_idx, int current_frame);
    void set_default_viewport_and_scissor(VkCommandBuffer &command_buffer);
    void end_render_pass_and_command_buffer(VkCommandBuffer command_buffer);
    
    // Draw to the swapchain and perform sync
    void end_frame();
    
    void cleanup();

    // Returns the index of the created buffer
    size_t create_index_buffer(VkDeviceSize buffer_size);
    size_t create_vertex_buffer(VkDeviceSize buffer_size);
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

    int add_light(glm::mat4 mvp, int type=0);
    void render_shadow_maps(VkCommandBuffer command_buffer, std::vector<Engine::Model> &models);

    template<typename T>
    size_t create_uniform_group(uint32_t binding, VkShaderStageFlags stage_flags) {
        if(binding == 0)
            throw std::runtime_error("Binding 0 is reserved for teh shadowmap in the frag shader");
        size_t ub_size = sizeof(T);
        size_t first_uniform_buffer_idx = create_uniform_buffer(ub_size);

        VkDescriptorSetLayoutBinding ubo_layout_binding{};
        ubo_layout_binding.binding = binding;
        ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding.descriptorCount = 1;
        ubo_layout_binding.stageFlags = stage_flags;
        ubo_layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorBindingFlags ubo_binding_flags = 0; 
        add_descriptor_set_layout_binding(ubo_layout_binding, ubo_binding_flags);

        UniformBufferGroup new_ubg{};
        new_ubg.m_base_index = first_uniform_buffer_idx;
        new_ubg.m_binding = binding;
        new_ubg.m_size = ub_size;

        m_uniforms.push_back(new_ubg);

        return m_uniforms.size() - 1;
    }
    void update_uniform_group(size_t idx, void* data);

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
    void create_descriptor_sets();
    void create_descriptor_set_layout();

    size_t create_uniform_buffer(VkDeviceSize buffer_size);
    void destroy_buffer(int buffer_idx);
    void destroy_pipeline(int pipeline_idx);

    void create_depth_resources();
    void create_color_resources();
    VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    bool has_stencil_component(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
    VkSampleCountFlagBits get_max_usable_sample_count();
    
    void initialize_lights();


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
    std::vector<UniformBufferGroup> m_uniforms;

    std::vector<TextureImage> m_textures;
    std::vector<TextureImageArray> m_texture_arrays;
    
    std::vector<Pipeline*> m_pipelines;

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

    // for lights
    Pipeline* m_shadow_pipeline;
    VkRenderPass m_shadow_render_pass = VK_NULL_HANDLE;
    ShadowMapImage m_shadow_map_image;
    std::vector<Light> m_lights;
};
}