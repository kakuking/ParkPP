#include <engine/pipeline.h>
#include <engine/renderer.h>
#include <engine/models.h>
#include <iostream>

#include <chrono>
#include <fmt/format.h>

namespace Engine {
class DefaultPipeline: public Pipeline {
    void create_pipeline(Renderer &device, VkFormat image_format) override {
        PipelineBuilder builder;
        std::vector<VkDescriptorSetLayout> layout = {device.get_descriptor_set_layout()};

        builder.create_pipeline_layout(device, layout.data());

        builder.create_render_pass(device.m_dispatch, device.get_swapchain());
        
        builder.set_shaders(device, "shaders/shader.vert.spv", "shaders/shader.frag.spv");
        builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
        builder.set_color_attachment_format(image_format);
        builder.enable_culling(VK_CULL_MODE_BACK_BIT);
        builder.disable_blending();
        builder.disable_depth_test();

        auto bind_desc = Vertex::get_binding_description();
        auto attr_desc = Vertex::get_attribute_description();

        builder.set_vertex_binding_and_attrs(bind_desc, attr_desc);

        m_data = builder.build(device);
    }

    void destroy_pipeline(vkb::DispatchTable &dispatch_table) override {
        m_data.frag_shader.destroy_shader(dispatch_table);
        m_data.vert_shader.destroy_shader(dispatch_table);
        dispatch_table.destroyRenderPass(m_data.render_pass, nullptr);
        dispatch_table.destroyPipelineLayout(m_data.pipeline_layout, nullptr);
        dispatch_table.destroyPipeline(m_data.pipeline, nullptr);
    }
};
}

int main() {    
    // Engine::Renderer renderer;
    Engine::Renderer device;
    Engine::DefaultPipeline pipeline;
    device.initialize_vulkan();
    
    size_t vb_size = sizeof(Engine::vertices[0]) * Engine::vertices.size();
    uint32_t v_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    uint32_t memory_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    size_t vertex_buffer_idx = device.create_buffer(vb_size, v_usage, memory_props);

    size_t ib_size = sizeof(Engine::indices[0]) * Engine::indices.size();
    uint32_t i_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    size_t index_buffer_idx = device.create_buffer(ib_size, i_usage, memory_props);

    size_t ub_size = sizeof(Engine::UniformBufferObject);
    uint32_t u_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    size_t first_uniform_buffer_idx = device.create_buffer(ub_size, u_usage, memory_props, true);
    
    device.update_buffer(vertex_buffer_idx, (void*)Engine::vertices.data(), vb_size);
    device.update_buffer(index_buffer_idx, (void*)Engine::indices.data(), ib_size);
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        Engine::UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));

        ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));

        ubo.proj = glm::perspective(glm::radians(45.f), device.get_swapchain_extent().width / (float) device.get_swapchain_extent().height, 0.f, 10.f);

        ubo.proj[1][1] *= -1;
        device.update_buffer(first_uniform_buffer_idx + i, &ubo, sizeof(ubo));
    }

    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags ubo_binding_flags = 0; 
    device.add_descriptor_set_layout_binding(ubo_layout_binding, ubo_binding_flags);

    std::vector<Engine::Pipeline*> pipelines = {&pipeline};
    std::vector<std::vector<uint32_t>> uniform_buffer_indices;
    std::vector<std::vector<uint32_t>> uniform_buffer_sizes;
    
    std::vector<uint32_t> temp_indices, temp_bindings;
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        temp_indices.push_back((uint32_t)first_uniform_buffer_idx);
        temp_bindings.push_back((uint32_t) ub_size);
    }
    uniform_buffer_indices.push_back(temp_indices);
    uniform_buffer_sizes.push_back(temp_bindings);

    device.initialize(pipelines, uniform_buffer_indices, uniform_buffer_sizes);

    // std::cout << "Intialized device!\n";
    
    int current_frame = 0;
    uint32_t image_index = 0;
    VkCommandBuffer command_buffer;

    using clock = std::chrono::high_resolution_clock;
    auto previous_frame_time = clock::now();
    float total_time = 0.0;

    while(device.begin_frame(current_frame, image_index, command_buffer)) {
        auto current_time = clock::now();
        float delta_time = std::chrono::duration<float>(current_time - previous_frame_time).count();
        previous_frame_time = current_time;
        total_time += delta_time;
        
        if (false) {
            double fps = delta_time > 0.0 ? 1.0 / delta_time: 0.0;
            fmt::println("{}", fps);
        }

        Engine::UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.f), total_time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));

        ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 1.1f + sinf(total_time)), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));

        ubo.proj = glm::perspective(glm::radians(45.f), device.get_swapchain_extent().width / (float) device.get_swapchain_extent().height, 0.f, 10.f);

        ubo.proj[1][1] *= -1;
        device.update_buffer(first_uniform_buffer_idx + current_frame, &ubo, sizeof(ubo));

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = device.get_render_pass();
        render_pass_info.framebuffer = device.get_framebuffer(image_index);
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = device.get_swapchain_extent();

        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;

        device.m_dispatch.cmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        device.m_dispatch.cmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_pipeline());

        VkBuffer vertex_buffers[] = {device.get_buffer(vertex_buffer_idx)};
        VkDeviceSize offsets[] = {0};

        VkBuffer index_buffer = device.get_buffer(index_buffer_idx);

        device.m_dispatch.cmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

        device.m_dispatch.cmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);

        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = static_cast<float>(device.get_swapchain_extent().width);
        viewport.height = static_cast<float>(device.get_swapchain_extent().height);
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        device.m_dispatch.cmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = device.get_swapchain_extent();
        device.m_dispatch.cmdSetScissor(command_buffer, 0, 1, &scissor);

        VkDescriptorSet cur_ds = device.get_descriptor_set(current_frame);
        device.m_dispatch.cmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_pipeline_layout(), 0, 1, &cur_ds, 0, nullptr);

        device.m_dispatch.cmdDrawIndexed(command_buffer, static_cast<uint32_t>(Engine::indices.size()), 1, 0, 0, 0);

        device.m_dispatch.cmdEndRenderPass(command_buffer);

        if(device.m_dispatch.endCommandBuffer(command_buffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffers!");

        device.end_frame();
    }

    device.cleanup();

    return 0;
}