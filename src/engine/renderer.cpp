#include <engine/renderer.h>
#include <fmt/format.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>


namespace Engine {

void Renderer::run() {
    m_window.init_window();

    initialize();
    loop();
    cleanup();
}

void Renderer::initialize() {
    std::cout << "Init\n";
    // std::cout << "Creating instance\n";
    create_instance();
    // std::cout << "Creating surface\n";
    create_surface();
    // std::cout << "Creating physical device\n";
    pick_physical_device();
    // std::cout << "Creating logical device\n";
    create_logical_device();
    // std::cout << "Creating swapchains\n";
    create_swapchains();
    // std::cout << "Creating pipeline\n";
    create_graphics_pipeline();
    // std::cout << "Creating framebuffers\n";
    create_framebuffers();
    // std::cout << "Creating command pool\n";
    create_command_pool();
    // std::cout << "Creating vertex buffer\n";
    create_vertex_buffer();
    // std::cout << "Creating index buffer\n";
    create_index_buffer();
    // std::cout << "Creating uniform buffer\n";
    create_uniform_buffers();
    // std::cout << "Creating Descriptor pool\n";
    create_descriptor_pool();
    // std::cout << "Creating Descriptor sets\n";
    create_descriptor_sets();
    // std::cout << "Creating command buffer\n";
    create_command_buffers();
    // std::cout << "Creating sync objects\n";
    create_sync_objects();
}


void Renderer::loop() {
    while(!glfwWindowShouldClose(m_window.get_window())){
        glfwPollEvents();
        draw_frame();
    }

    m_dispatch.deviceWaitIdle();
}

void Renderer::cleanup() {
    std::cout << "Cleaning up\n";

    cleanup_swapchain();

    m_dispatch.destroyDescriptorPool(m_descriptor_pool, nullptr);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_dispatch.destroyBuffer(m_uniform_buffers[i], nullptr);
        m_dispatch.freeMemory(m_uniform_buffers_memory[i], nullptr);
    }

    m_dispatch.destroyBuffer(m_vertex_buffer, nullptr);
    m_dispatch.freeMemory(m_vertex_buffer_memory, nullptr);
    m_dispatch.destroyBuffer(m_index_buffer, nullptr);
    m_dispatch.freeMemory(m_index_buffer_memory, nullptr);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_dispatch.destroySemaphore(m_image_available_semaphores[i], nullptr);
        m_dispatch.destroySemaphore(m_render_finished_semaphores[i], nullptr);
        m_dispatch.destroyFence(m_in_flight_fences[i], nullptr);
    }

    m_dispatch.destroyCommandPool(m_command_pool, nullptr);

    m_graphics_pipeline.destroy_pipeline(m_dispatch);
    
    vkb::destroy_device(m_device);
    vkb::destroy_surface(m_instance, m_surface);
    vkb::destroy_instance(m_instance);
    m_window.cleanup();
}

void Renderer::cleanup_swapchain() {
    for(auto framebuffer: m_swapchain_framebuffers)
        m_dispatch.destroyFramebuffer(framebuffer, nullptr);

    m_swapchain.destroy_image_views(m_swapchain_image_views);
    vkb::destroy_swapchain(m_swapchain);
}

void Renderer::create_instance() {
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
    m_instance_dispatch = m_instance.make_table();
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

void Renderer::create_graphics_pipeline() {
    m_graphics_pipeline.create_pipeline(m_dispatch, m_swapchain, "./shaders/shader.vert.spv", "./shaders/shader.frag.spv");
}

void Renderer::create_framebuffers() {
    m_swapchain_framebuffers.resize(m_swapchain_image_views.size());

    for(size_t i = 0; i < m_swapchain_image_views.size(); i++) {
        VkImageView attachments[] = {
            m_swapchain_image_views[i]
        };

        VkFramebufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = m_graphics_pipeline.get_render_pass();
        info.attachmentCount = 1;
        info.pAttachments = attachments;
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

void Renderer::create_command_buffers() {
    m_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = m_command_pool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = static_cast<uint32_t>(m_command_buffers.size());

    if(m_dispatch.allocateCommandBuffers(&info, m_command_buffers.data()) != VK_SUCCESS) 
        throw std::runtime_error("Could not allocate command buffer");
}

void Renderer::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    if(m_dispatch.beginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) 
        throw std::runtime_error("Failed to begin recording command buffer!");

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m_graphics_pipeline.get_render_pass();
    render_pass_info.framebuffer = m_swapchain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = m_swapchain.extent;

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    m_dispatch.cmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    m_dispatch.cmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline.get_pipeline());

    VkBuffer vertex_buffers[] = {m_vertex_buffer};
    VkDeviceSize offsets[] = {0};

    m_dispatch.cmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

    m_dispatch.cmdBindIndexBuffer(command_buffer, m_index_buffer, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(m_swapchain.extent.width);
    viewport.height = static_cast<float>(m_swapchain.extent.height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    m_dispatch.cmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain.extent;
    m_dispatch.cmdSetScissor(command_buffer, 0, 1, &scissor);

    m_dispatch.cmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline.get_pipeline_layout(), 0, 1, &m_descriptor_sets[m_current_frame], 0, nullptr);

    m_dispatch.cmdDrawIndexed(command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    m_dispatch.cmdEndRenderPass(command_buffer);

    if(m_dispatch.endCommandBuffer(command_buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffers!");
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

void Renderer::draw_frame() {
    m_dispatch.waitForFences(1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_idx;
    VkResult result = m_dispatch.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &image_idx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swap_chain();
        return;
    } else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain images!");
    }

    m_dispatch.resetFences(1, &m_in_flight_fences[m_current_frame]);

    m_dispatch.resetCommandBuffer(m_command_buffers[m_current_frame], 0);

    update_uniform_buffer(m_current_frame);

    record_command_buffer(m_command_buffers[m_current_frame], image_idx);

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
    present_info.pImageIndices = &image_idx;
    present_info.pResults = nullptr;

    result = m_dispatch.queuePresentKHR(m_present_queue, &present_info);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.m_frame_buffer_resized)
        recreate_swap_chain();
    else if(result != VK_SUCCESS)
        throw std::runtime_error("Failed to present swapchain image!");

    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::recreate_swap_chain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window.get_window(), &width, &height);
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window.get_window(), &width, &height);
        glfwWaitEvents();
    }
    
    m_dispatch.deviceWaitIdle();
    cleanup_swapchain();

    create_swapchains();
    create_framebuffers();
}

void Renderer::create_buffer(VkDeviceSize buffer_size, uint32_t usage, uint32_t memory_props, VkBuffer &buffer, VkDeviceMemory  &buffer_memory) {
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
}

// void Renderer::create_vertex_buffer(VkDeviceSize vertex_buffer_size, void* data, bool persistent_buffer = false) {
void Renderer::create_vertex_buffer() {
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size(); 

    create_buffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_vertex_buffer, m_vertex_buffer_memory);
    
    m_dispatch.bindBufferMemory(m_vertex_buffer, m_vertex_buffer_memory, 0);

    void* data;
    m_dispatch.mapMemory(m_vertex_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), (size_t) buffer_size);
    m_dispatch.unmapMemory(m_vertex_buffer_memory);
}

uint32_t Renderer::find_memory_type(uint32_t filter, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties mem_prop;
    m_instance_dispatch.getPhysicalDeviceMemoryProperties(m_physical_device, &mem_prop);

    for(uint32_t i = 0; i < mem_prop.memoryTypeCount; i++) 
        if((filter & (1 << i)) && (mem_prop.memoryTypes[i].propertyFlags == props))
            return i;
    
    throw std::runtime_error("Failed to find suitable memory type!");
}

void Renderer::create_index_buffer() {
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();

    create_buffer(buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_index_buffer, m_index_buffer_memory);
    
        
    m_dispatch.bindBufferMemory(m_index_buffer, m_index_buffer_memory, 0);
    void* data;
    m_dispatch.mapMemory(m_index_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, indices.data(), (size_t) buffer_size);
    m_dispatch.unmapMemory(m_index_buffer_memory);
}

void Renderer::create_uniform_buffers() {
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);

    m_uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniform_buffers[i], m_uniform_buffers_memory[i]);

        m_dispatch.bindBufferMemory(m_uniform_buffers[i], m_uniform_buffers_memory[i], 0);
        m_dispatch.mapMemory(m_uniform_buffers_memory[i], 0, buffer_size, 0, &m_uniform_buffers_mapped[i]);
    }
}

void Renderer::update_uniform_buffer(uint32_t current_image) {
    static auto start_time = std::chrono::high_resolution_clock::now();

    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));

    ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));

    ubo.proj = glm::perspective(glm::radians(45.f), m_swapchain.extent.width / (float) m_swapchain.extent.height, 0.f, 10.f);

    ubo.proj[1][1] *= -1;

    memcpy(m_uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}

void Renderer::create_descriptor_pool() {
    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if(m_dispatch.createDescriptorPool(&pool_info, nullptr, &m_descriptor_pool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create a descriptor pool!");
}

void Renderer::create_descriptor_sets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_graphics_pipeline.get_descriptor_set_layout());
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    alloc_info.pSetLayouts = layouts.data();

    m_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
    if(m_dispatch.allocateDescriptorSets(&alloc_info, m_descriptor_sets.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor sets!");

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = m_uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = m_descriptor_sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;

        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = nullptr;
        descriptor_write.pTexelBufferView = nullptr;

        m_dispatch.updateDescriptorSets(1, &descriptor_write, 0, nullptr);
    } 
}

};