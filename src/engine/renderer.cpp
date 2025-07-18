#include <engine/renderer.h>
#include <engine/pipeline.h>

#include <engine/image.h>
#include <engine/models.h>
#include <engine/pipeline.h>

#include <iostream>
#include <fmt/format.h>

namespace Engine {

void Renderer::initialize_vulkan() {
    // std::cout << "Creating window!\n";
    m_window.init_window();
    // std::cout << "Creating instancs!\n";
    create_instance();
    // std::cout << "Creating surface!\n";
    create_surface();
    // std::cout << "Creating phy dev!\n";
    pick_physical_device();
    // std::cout << "Creating log dev!\n";
    create_logical_device();
    // std::cout << "Creating swapchain!\n";
    create_swapchains();
    // create_pipeline();
    m_shadow_pipeline = new ShadowPipeline();
    m_shadow_pipeline->create_pipeline(*this, m_swapchain.image_format);

    m_shadow_render_pass = m_shadow_pipeline->get_render_pass();

    VkDescriptorSetLayoutBinding shadow_map_binding{};
    shadow_map_binding.binding = 0;
    shadow_map_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadow_map_binding.descriptorCount = 1;  // One descriptor for the whole array
    shadow_map_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    shadow_map_binding.pImmutableSamplers = nullptr;

    add_descriptor_set_layout_binding(shadow_map_binding, 0);
}

void Renderer::initialize(
    std::vector<Pipeline*> pipelines
) {
    // std::cout << "setting pipelines window!\n";
    m_pipelines = pipelines;

    create_command_pool();

    create_color_resources();
    create_depth_resources();

    for(TextureImage &tex: m_textures)
        Image::initialize_texture_image(*this, tex);

    for(TextureImageArray &tex: m_texture_arrays)
        Image::initialize_texture_image_array(*this, tex);
    
    initialize_lights();

    // std::cout << "Creating desc pool!\n";
    create_descriptor_pool();
    // std::cout << "Creating desc set layout!\n";
    create_descriptor_set_layout();
    // std::cout << "Creating desc sets!\n";
    create_descriptor_sets();

    // std::cout << "Creating pipelines!\n";
    for(Pipeline* pipeline: m_pipelines)
        pipeline->create_pipeline(*this, m_swapchain.image_format);

    // std::cout << "getting render pass!\n";
    m_render_pass = m_pipelines[0]->get_render_pass();

    // std::cout << "Creating fbs!\n";
    create_framebuffers();

    create_command_buffer();
    // std::cout << "Creating sos!\n";
    create_sync_objects();
}

bool Renderer::begin_frame(int &current_frame, uint32_t &image_index, VkCommandBuffer &out_buffer) {
    // std::cout << "Window should close\n";
    if (window_should_close())
        return false;
    
    m_window.poll_events();
    
    // std::cout << "Waiting for fences\n";
    m_dispatch.waitForFences(1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);
    
    // uint32_t image_idx;
    // std::cout << "acq image\n";
    VkResult result = m_dispatch.acquireNextImageKHR(
        m_swapchain, UINT64_MAX, m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swap_chain();
        return false;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    // std::cout << "reset fences\n";
    m_dispatch.resetFences(1, &m_in_flight_fences[m_current_frame]);
    // std::cout << "reset cb\n";
    m_dispatch.resetCommandBuffer(m_command_buffers[m_current_frame], 0);

    // std::cout << "setting ob\n";
    out_buffer = m_command_buffers[m_current_frame];
    // std::cout << "current frame\n";
    current_frame = m_current_frame;
    // std::cout << "image index\n";
    m_current_image_index = image_index;

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    if (m_dispatch.beginCommandBuffer(out_buffer, &begin_info) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer!");

    return true;
}

void Renderer::begin_render_pass(VkCommandBuffer &command_buffer, uint32_t image_index) {
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = get_render_pass();
    render_pass_info.framebuffer = get_framebuffer(image_index);
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = get_swapchain_extent();

    std::vector<VkClearValue> clear_colors(2);
    clear_colors[0].color = {{0.f, 0.f, 0.f, 1.f}};
    clear_colors[1].depthStencil = {1.f, 0};

    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_colors.size());
    render_pass_info.pClearValues = clear_colors.data();

    m_dispatch.cmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::bind_pipeline_and_descriptors(VkCommandBuffer &command_buffer, int pipeline_idx, int current_frame) {
    m_dispatch.cmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[pipeline_idx]->get_pipeline());

    VkDescriptorSet cur_ds = get_descriptor_set(current_frame);
    m_dispatch.cmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[pipeline_idx]->get_pipeline_layout(), 0, 1, &cur_ds, 0, nullptr);

}

void Renderer::set_default_viewport_and_scissor(VkCommandBuffer &command_buffer) {
    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(get_swapchain_extent().width);
    viewport.height = static_cast<float>(get_swapchain_extent().height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    m_dispatch.cmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = get_swapchain_extent();
    m_dispatch.cmdSetScissor(command_buffer, 0, 1, &scissor);
}

void Renderer::end_render_pass_and_command_buffer(VkCommandBuffer command_buffer) {
    m_dispatch.cmdEndRenderPass(command_buffer);

    if(m_dispatch.endCommandBuffer(command_buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffers!");
}


void Renderer::end_frame() {
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore wait_semaphores[] = {m_image_available_semaphores[m_current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffers[m_current_frame];

    VkSemaphore signal_semaphores[] = {m_render_finished_semaphores[m_current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if(m_dispatch.queueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit draw command buffer!");
    
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {m_swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &m_current_image_index;
    present_info.pResults = nullptr;

    VkResult result = m_dispatch.queuePresentKHR(m_present_queue, &present_info);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.m_frame_buffer_resized) {
        m_window.m_frame_buffer_resized = false;
        recreate_swap_chain();
    } else if(result != VK_SUCCESS)
        throw std::runtime_error("Failed to present swapchain image!");

    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::cleanup() {
    m_dispatch.deviceWaitIdle();

    m_shadow_map_image.cleanup(m_dispatch);

    for(auto &light: m_lights)
        light.cleanup(m_dispatch);
    
    // std::cout << "Cleaning up swapchain!\n";
    cleanup_swapchain();
    // std::cout << "Cleaning up dp\n";
    m_dispatch.destroyDescriptorPool(m_descriptor_pool, nullptr);
    // std::cout << "Cleaning up dsl\n";
    m_dispatch.destroyDescriptorSetLayout(m_descriptor_set_layout, nullptr);

    for(int i = 0; i < m_textures.size(); i++)
        m_textures[i].cleanup(m_dispatch);

    for(int i = 0; i < m_texture_arrays.size(); i++)
        m_texture_arrays[i].cleanup(m_dispatch);

    // std::cout << "Cleaning up b\n";
    // destroy buffers
    for(int i = 0; i < m_buffers.size(); i++)
        destroy_buffer(i);

    // std::cout << "Cleaning up sos\n";
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_dispatch.destroySemaphore(m_image_available_semaphores[i], nullptr);
        m_dispatch.destroySemaphore(m_render_finished_semaphores[i], nullptr);
        m_dispatch.destroyFence(m_in_flight_fences[i], nullptr);
    }

    // std::cout << "Cleaning up cp\n";
    m_dispatch.destroyCommandPool(m_command_pool, nullptr);

    // std::cout << "Cleaning up p\n";
    // destroy pipelines
    for(int i = 0; i < m_pipelines.size(); i++)
        destroy_pipeline(i);

    m_shadow_pipeline->destroy_pipeline(m_dispatch);

    // std::cout << "Cleaning up dev\n";
    vkb::destroy_device(m_device);
    // std::cout << "Cleaning up surface\n";
    vkb::destroy_surface(m_instance, m_surface);
    // std::cout << "Cleaning up inst\n";
    vkb::destroy_instance(m_instance);
    // std::cout << "Cleaning up widnow\n";
    m_window.cleanup();

}

// If it is per-frame, create MAX_FRAMES_IN_FLIGHT copies of it, and send the idx of the first one :)
size_t Renderer::create_buffer(VkDeviceSize buffer_size, uint32_t usage, uint32_t memory_props, bool per_frame) {
    size_t ret = m_buffers.size();
    uint32_t count = per_frame ? MAX_FRAMES_IN_FLIGHT : 1;

    for(uint32_t i = 0; i < count; i++) {
        VkBuffer buffer;
        VkDeviceMemory buffer_memory;
    
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = buffer_size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
        if(m_dispatch.createBuffer(&buffer_info, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to create buffer!");
    
        VkMemoryRequirements mem_req{};
        m_dispatch.getBufferMemoryRequirements(buffer, &mem_req);
    
        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_req.size;
        alloc_info.memoryTypeIndex = find_memory_type(mem_req.memoryTypeBits, memory_props);
    
        if(m_dispatch.allocateMemory(&alloc_info, nullptr, &buffer_memory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        
        m_dispatch.bindBufferMemory(buffer, buffer_memory, 0);
    
        m_buffers.push_back(buffer);
        m_buffer_memories.push_back(buffer_memory);
    }

    return ret;
}

size_t Renderer::create_index_buffer(VkDeviceSize buffer_size) {
    uint32_t usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    uint32_t memory_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return create_buffer(buffer_size, usage, memory_props);
}

size_t Renderer::create_vertex_buffer(VkDeviceSize buffer_size) {
    uint32_t usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    uint32_t memory_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    
    return create_buffer(buffer_size, usage, memory_props);
}

size_t Renderer::create_uniform_buffer(VkDeviceSize buffer_size) {
    uint32_t usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    uint32_t memory_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return create_buffer(buffer_size, usage, memory_props, true);
}

void Renderer::update_buffer(size_t buffer_idx, void* src_data, size_t src_data_size) {
    // VkBuffer &buffer = m_buffers[buffer_idx];
    VkDeviceMemory &buffer_memory = m_buffer_memories[buffer_idx];
    void* data;

    m_dispatch.mapMemory(buffer_memory, 0, src_data_size, 0, &data);
    memcpy(data, src_data, src_data_size);
    m_dispatch.unmapMemory(buffer_memory);
}

void Renderer::destroy_buffer(int buffer_idx) {
    if(buffer_idx >= m_buffers.size())
        throw std::runtime_error("The index of the buffer is outside range of indices");

        VkBuffer &buffer = m_buffers[buffer_idx];
        VkDeviceMemory &buffer_memory = m_buffer_memories[buffer_idx];

        m_dispatch.freeMemory(buffer_memory, nullptr);
        m_dispatch.destroyBuffer(buffer, nullptr);
}

VkCommandBuffer Renderer::begin_single_time_command() {
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = m_command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    m_dispatch.allocateCommandBuffers(&alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    m_dispatch.beginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void Renderer::end_single_time_command(VkCommandBuffer command_buffer) {
    m_dispatch.endCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    m_dispatch.queueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_dispatch.queueWaitIdle(m_graphics_queue);

    m_dispatch.freeCommandBuffers(m_command_pool, 1, &command_buffer);
}

void Renderer::destroy_pipeline(int pipeline_idx) {
    if(pipeline_idx >= m_pipelines.size())
        throw std::runtime_error("The index of the pipeline is outside range of indices");
    
    Pipeline* pipeline = m_pipelines[pipeline_idx];
    pipeline->destroy_pipeline(m_dispatch);
}

void Renderer::create_instance() {
    // Get extensions
    // std::cout << "Getting extns\n";

    std::vector<const char *> extensions = m_window.get_required_instance_extensions();
    
    // extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    // std::cout << "Getting sys info\n";
    auto system_info_ret = vkb::SystemInfo::get_system_info(0);
    if (!system_info_ret)
        throw std::runtime_error("Could not get system info");
    auto system_info = system_info_ret.value();

    if(!system_info.validation_layers_available) {
        std::cout << "No validation layers available!\n";
    }

    // std::cout << "Creating instance builder\n";
    vkb::InstanceBuilder builder;
    vkb::Result<vkb::Instance> instance_ret = builder.set_app_name("Pogger Park")
                                .require_api_version(1, 2, 0)
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
    m_instance_dispatch = m_instance.make_table();

}

void Renderer::create_surface() {
    VkResult res = m_window.create_window_surface(m_instance, m_surface);
    if(res != VK_SUCCESS)
        throw std::runtime_error("Failed to create a window surface!");
}

void Renderer::pick_physical_device() {
    vkb::PhysicalDeviceSelector selector(m_instance);

    VkPhysicalDeviceFeatures required_features{};
    required_features.samplerAnisotropy = VK_TRUE;

    // Here you would ask for requirements normally
    auto selector_ret = selector.require_present().set_required_features(required_features).set_surface(m_surface).select();

    if(!selector_ret) {
        std::cout << "Error: " << selector_ret.error() << "\n\n";
        throw std::runtime_error("Could not select a physical device!");
    }
    
    m_physical_device = selector_ret.value();
    m_instance_dispatch.getPhysicalDeviceProperties(m_physical_device, &m_physical_device_properties);

    m_msaa_samples = get_max_usable_sample_count();
}

void Renderer::create_logical_device() {
    vkb::DeviceBuilder builder(m_physical_device);
    auto builder_ret = builder.build();
    if(!builder_ret)
        throw std::runtime_error("Could not create logical device");
    
    m_device = builder_ret.value();
    m_dispatch = m_device.make_table();

    auto graphics_queue_ret = m_device.get_queue(vkb::QueueType::graphics);
    if(!graphics_queue_ret)
        throw std::runtime_error("Could not find a graphics queue");
        
    m_graphics_queue = graphics_queue_ret.value();

    auto graphics_queue_idx_ret = m_device.get_queue_index(vkb::QueueType::graphics);
    if(!graphics_queue_idx_ret)
        throw std::runtime_error("Could not find a graphics queue");

    m_graphics_queue_idx = graphics_queue_idx_ret.value();
    
    auto present_queue_ret = m_device.get_queue(vkb::QueueType::present);
    if(!present_queue_ret)
    throw std::runtime_error("Could not find a present queue");
    
    m_present_queue = present_queue_ret.value();

    auto present_queue_idx_ret = m_device.get_queue_index(vkb::QueueType::present);
    if(!present_queue_idx_ret)
        throw std::runtime_error("Could not find a present queue");

    m_present_queue_idx = present_queue_idx_ret.value();
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
}

void Renderer::create_framebuffers() {
    m_swapchain_framebuffers.resize(m_swapchain_image_views.size());

    for(size_t i = 0; i < m_swapchain_image_views.size(); i++) {
        std::vector<VkImageView> attachments = {
            m_color_image.m_image_view,
            m_depth.m_image_view,
            m_swapchain_image_views[i]
        };

        VkFramebufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = m_render_pass;
        info.attachmentCount = static_cast<uint32_t>(attachments.size());
        info.pAttachments = attachments.data();
        info.width = m_swapchain.extent.width;
        info.height = m_swapchain.extent.height;
        info.layers = 1;

        if(m_dispatch.createFramebuffer(&info, nullptr, &m_swapchain_framebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create a framebuffer!");
    }
}

void Renderer::create_command_pool() {
    VkCommandPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = m_graphics_queue_idx;

    if(m_dispatch.createCommandPool(&info, nullptr, &m_command_pool) != VK_SUCCESS) 
        throw std::runtime_error("Failed to create command pool");
}

void Renderer::create_command_buffer() {
    m_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = m_command_pool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = static_cast<uint32_t>(m_command_buffers.size());

    if (m_dispatch.allocateCommandBuffers(&info, m_command_buffers.data()) != VK_SUCCESS) 
        throw std::runtime_error("Could not allocate command buffer");
}

void Renderer::copy_buffer_to_image(size_t buffer_idx, VkImage &image, uint32_t width, uint32_t height, uint32_t layer) {
    VkCommandBuffer command_buffer = begin_single_time_command();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = layer;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    m_dispatch.cmdCopyBufferToImage(
        command_buffer,
        m_buffers[buffer_idx],
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    end_single_time_command(command_buffer);
}

void Renderer::recreate_swap_chain() {
    int width = 0, height = 0;
    m_window.get_framebuffer_size(width, height);
    while(width == 0 || height == 0) {
        m_window.get_framebuffer_size(width, height);
        m_window.wait_events();
    }
    
    m_dispatch.deviceWaitIdle();
    cleanup_swapchain();

    create_swapchains();
    create_color_resources();
    create_depth_resources();
    create_framebuffers();
}

void Renderer::cleanup_swapchain() {
    for(auto framebuffer: m_swapchain_framebuffers)
        m_dispatch.destroyFramebuffer(framebuffer, nullptr);

    m_depth.cleanup(m_dispatch);
    m_color_image.cleanup(m_dispatch);
    m_swapchain.destroy_image_views(m_swapchain_image_views);
    vkb::destroy_swapchain(m_swapchain);
}

// TODO - add functionality for pool to automatically be size of descriptors added
void Renderer::create_descriptor_pool() {
    std::vector<VkDescriptorPoolSize> pool_sizes(2, {});
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = static_cast<uint32_t>(num_buffer_descriptor_sets * MAX_FRAMES_IN_FLIGHT);
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = static_cast<uint32_t>((num_image_descriptor_sets + 1) * MAX_FRAMES_IN_FLIGHT); // 1 for the shadow map sampler

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = static_cast<uint32_t>((num_buffer_descriptor_sets + num_image_descriptor_sets) * MAX_FRAMES_IN_FLIGHT);

    if(m_dispatch.createDescriptorPool(&pool_info, nullptr, &m_descriptor_pool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create a descriptor pool!");
}

uint32_t Renderer::find_memory_type(uint32_t filter, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties mem_prop;
    m_instance_dispatch.getPhysicalDeviceMemoryProperties(m_physical_device, &mem_prop);

    for(uint32_t i = 0; i < mem_prop.memoryTypeCount; i++) 
        if((filter & (1 << i)) && (mem_prop.memoryTypes[i].propertyFlags == props))
            return i;
    
    throw std::runtime_error("Failed to find suitable memory type!");
}

void Renderer::add_texture(std::string filename, uint32_t binding) {
    if(binding == 0)
        throw std::runtime_error("Binding 0 is reserved for teh shadowmap in the frag shader");
    TextureImage tex;
    tex = Image::create_texture_image(filename);
    m_textures.push_back(tex);

    VkDescriptorSetLayoutBinding texture_layout_binding{};
    texture_layout_binding.binding = binding;
    texture_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_layout_binding.descriptorCount = 1;
    texture_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    texture_layout_binding.pImmutableSamplers = nullptr;

    add_descriptor_set_layout_binding(texture_layout_binding, 0);
}

void Renderer::add_texture_array(std::vector<std::string> filename, uint32_t width, uint32_t height, uint32_t layer_count, uint32_t binding) {
    if(binding == 0)
        throw std::runtime_error("Binding 0 is reserved for teh shadowmap in the frag shader");

    TextureImageArray tex;
    tex = Image::create_texture_image_array(filename, width, height, layer_count);

    m_texture_arrays.push_back(tex);

    VkDescriptorSetLayoutBinding texture_array_binding{};
    texture_array_binding.binding = binding;
    texture_array_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_array_binding.descriptorCount = 1;  // One descriptor for the whole array
    texture_array_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    texture_array_binding.pImmutableSamplers = nullptr;

    add_descriptor_set_layout_binding(texture_array_binding, 0);
}


void Renderer::create_sync_objects() {
    m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo sem_info{};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if(
            m_dispatch.createSemaphore(&sem_info, nullptr, &m_image_available_semaphores[i]) != VK_SUCCESS || 
            m_dispatch.createSemaphore(&sem_info, nullptr, &m_render_finished_semaphores[i]) != VK_SUCCESS || 
            m_dispatch.createFence(&fence_info, nullptr, &m_in_flight_fences[i]) != VK_SUCCESS)
            throw std::runtime_error("Could not create semaphores and fence!");
    }
    
}

bool Renderer::window_should_close() {
    return m_window.window_should_close();
}

void Renderer::add_descriptor_set_layout_binding(VkDescriptorSetLayoutBinding binding, VkDescriptorBindingFlags binding_flag) { 
    m_descriptor_bindings.push_back(binding); 
    m_descriptor_binding_flags.push_back(binding_flag); 
    if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        num_buffer_descriptor_sets++;
    else if(binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
        num_image_descriptor_sets++;
}

void Renderer::create_descriptor_set_layout() {

    // for (int i = 0; i < m_descriptor_bindings.size(); i++)
    //    fmt::println("Binding {}", m_descriptor_bindings[i].binding);

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(m_descriptor_bindings.size());
    layout_info.pBindings = m_descriptor_bindings.data();

    if(m_dispatch.createDescriptorSetLayout(&layout_info, nullptr, &m_descriptor_set_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout!");
    
}

// [binding][frame]
void Renderer::create_descriptor_sets() {
    std::vector<std::vector<uint32_t>> uniform_buffer_indices; 
    std::vector<std::vector<uint32_t>> uniform_buffer_sizes;
    for(int i = 0; i < m_uniforms.size(); i++) {
        std::vector<uint32_t> temp_indices, temp_sizes;
        for(int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            temp_indices.push_back((uint32_t) m_uniforms[i].m_base_index);
            temp_sizes.push_back((uint32_t) m_uniforms[i].m_size);
        }
        uniform_buffer_indices.push_back(temp_indices);
        uniform_buffer_sizes.push_back(temp_sizes);
    }

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptor_set_layout);
    
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    alloc_info.pSetLayouts = layouts.data();

    m_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
    if (m_dispatch.allocateDescriptorSets(&alloc_info, m_descriptor_sets.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor sets!");

    const size_t num_uniform_buffer_bindings = uniform_buffer_indices.size();
    const size_t num_images = m_textures.size();
    const size_t num_image_arrays = m_texture_arrays.size();

    for (size_t frame = 0; frame < MAX_FRAMES_IN_FLIGHT; ++frame) {
        std::vector<VkWriteDescriptorSet> descriptor_writes;
        std::vector<VkDescriptorBufferInfo> buffer_infos(num_uniform_buffer_bindings); // needs to persist per frame
        std::vector<VkDescriptorImageInfo> image_infos(num_images); // needs to persist per frame
        std::vector<VkDescriptorImageInfo> image_array_infos(num_image_arrays); // needs to persist per frame

        VkDescriptorImageInfo shadow_image_info{};
        shadow_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadow_image_info.imageView = m_shadow_map_image.m_image_view;  // <- your shadow map array view
        shadow_image_info.sampler = m_shadow_map_image.m_sampler;       // <- its sampler

        VkWriteDescriptorSet shadow_write{};
        shadow_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        shadow_write.dstSet = m_descriptor_sets[frame];
        shadow_write.dstBinding = 0;
        shadow_write.dstArrayElement = 0;
        shadow_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        shadow_write.descriptorCount = 1;
        shadow_write.pImageInfo = &shadow_image_info;

        descriptor_writes.push_back(shadow_write);

        for (size_t binding = 0; binding < num_uniform_buffer_bindings; ++binding) {
            buffer_infos[binding].buffer = m_buffers[uniform_buffer_indices[binding][frame]];
            buffer_infos[binding].offset = 0;
            buffer_infos[binding].range = uniform_buffer_sizes[binding][frame];

            VkWriteDescriptorSet descriptor_write{};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = m_descriptor_sets[frame];
            descriptor_write.dstBinding = static_cast<uint32_t>(binding + 1);
            descriptor_write.dstArrayElement = 0;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pBufferInfo = &buffer_infos[binding];

            descriptor_writes.push_back(descriptor_write);
        }

        for(size_t binding = 0; binding < num_images; binding++) {
            image_infos[binding].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_infos[binding].imageView = m_textures[binding].m_image_view;
            image_infos[binding].sampler = m_textures[binding].m_sampler;

            VkWriteDescriptorSet descriptor_write{};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = m_descriptor_sets[frame];
            descriptor_write.dstBinding = static_cast<uint32_t>(binding + 1 + num_uniform_buffer_bindings);
            descriptor_write.dstArrayElement = 0;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pImageInfo = &image_infos[binding];

            descriptor_writes.push_back(descriptor_write);
        }

        for (size_t i = 0; i < num_image_arrays; ++i) {
            image_array_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_array_infos[i].imageView = m_texture_arrays[i].m_image_view;
            image_array_infos[i].sampler = m_texture_arrays[i].m_sampler;

            VkWriteDescriptorSet descriptor_write{};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = m_descriptor_sets[frame];
            descriptor_write.dstBinding = static_cast<uint32_t>(num_uniform_buffer_bindings + num_images + i + 1); // <-- note offset
            descriptor_write.dstArrayElement = 0;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pImageInfo = &image_array_infos[i];

            descriptor_writes.push_back(descriptor_write);
        }

        // Use buffer_infos, which lives long enough for the call
        m_dispatch.updateDescriptorSets(static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
    }
}

VkFormat Renderer::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        m_instance_dispatch.getPhysicalDeviceFormatProperties(m_physical_device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat Renderer::find_depth_format() {
    return find_supported_format(
        {VK_FORMAT_D32_SFLOAT},//, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void Renderer::create_depth_resources() {
    m_depth = Image::create_depth_image(*this, m_swapchain.extent.width, m_swapchain.extent.height, find_depth_format());
}

VkSampleCountFlagBits Renderer::get_max_usable_sample_count() {
    VkSampleCountFlags counts = m_physical_device_properties.limits.framebufferColorSampleCounts & m_physical_device_properties.limits.framebufferDepthSampleCounts;
    
    if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}

void Renderer::create_color_resources() {
    VkFormat color_format = m_swapchain.image_format;

    m_color_image = Image::create_color_image(*this, m_swapchain.extent.width, m_swapchain.extent.height, color_format, m_msaa_samples);
}

// template<typename T>
// size_t Renderer::create_uniform_group(uint32_t binding, VkShaderStageFlags stage_flags)

void Renderer::update_uniform_group(size_t idx, void* data) {
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        update_buffer(m_uniforms[idx].m_base_index + i, data, m_uniforms[idx].m_size);
    }
}

int Renderer::add_light(glm::mat4 mvp, int type) {
    Light light{};
    light.mvp = mvp;

    m_lights.push_back(light);

    return static_cast<int>(m_lights.size()) - 1;
}

void Renderer::initialize_lights() {
    m_shadow_map_image = Image::create_shadow_map_image(*this, 1024, 1024, find_depth_format(), 32);

    for (uint32_t i = 0; i < m_lights.size(); ++i) {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = m_shadow_map_image.m_image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // single layer view
        view_info.format = find_depth_format();
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = i;  // one layer at a time
        view_info.subresourceRange.layerCount = 1;

        if (m_dispatch.createImageView(&view_info, nullptr, &m_lights[i].image_view) != VK_SUCCESS)
            throw std::runtime_error("Failed to create per-layer image view!");

        // m_lights[i].image_view = m_shadow_map_image.m_image_view;

    // for(int i = 0; i < m_lights.size(); i++) {
        std::vector<VkImageView> attachments = {
            m_lights[i].image_view
        };
        
        VkFramebufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = m_shadow_pipeline->get_render_pass();
        info.attachmentCount = static_cast<uint32_t>(attachments.size());
        info.pAttachments = attachments.data();
        info.width = 1024;
        info.height = 1024;
        info.layers = 1;
        
        if(m_dispatch.createFramebuffer(&info, nullptr, &m_lights[i].framebuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to create a framebuffer!");        
    }
}

void Renderer::render_shadow_maps(VkCommandBuffer command_buffer, std::vector<Engine::Model> &models) {
    if (m_lights.empty()) return;

    // VkFormat depth_format = find_depth_format();

    // Image::transition_image_layout(*this, m_shadow_map_image.m_image, depth_format, 
    //     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, command_buffer);

    struct LightModel {
        glm::mat4 mvp;
        glm::mat4 model;
    };

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m_shadow_render_pass;
    render_pass_info.framebuffer = VK_NULL_HANDLE;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = VkExtent2D{1024, 1024};

    std::vector<VkClearValue> clear_colors(1);
    clear_colors[0].depthStencil = {1.f, 0};

    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_colors.size());
    render_pass_info.pClearValues = clear_colors.data();

    for (Light& light : m_lights) {
        render_pass_info.framebuffer = light.framebuffer;

        m_dispatch.cmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        m_dispatch.cmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadow_pipeline->get_pipeline());        
        
        LightModel lm{};
        lm.mvp = light.mvp;

        for (const Engine::Model &model : models) {
            // Retreive vertex and index buffers =============================================================
            VkBuffer vertex_buffers[] = {get_buffer(model.vertex_buffer_idx)};
            VkDeviceSize offsets[] = {0};
            VkBuffer index_buffer = get_buffer(model.index_buffer_idx);
            
            // Bind vertex and index buffers
            m_dispatch.cmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
            m_dispatch.cmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

            // Set push constants ============================================================================
            lm.model = model.model_matrix;
            m_dispatch.cmdPushConstants(command_buffer, m_shadow_pipeline->get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LightModel), &lm);
            
            // draw call
            m_dispatch.cmdDrawIndexed(command_buffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
        }

        m_dispatch.cmdEndRenderPass(command_buffer);
    }

    // std::cout << "trans image from VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL to  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL\n";
    // Image::transition_image_layout(*this, m_shadow_map_image.m_image, depth_format, 
    //     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, command_buffer);
}

}