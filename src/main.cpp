#include <engine/pipeline.h>
#include <engine/renderer.h>

#include <game/default_pipeline.h>
#include <game/models.h>

#include <fmt/format.h>

#include <iostream>
#include <chrono>

int main() {    
    // Initializing Vulkan  =====================================================================================
    Engine::Renderer renderer;
    Game::DefaultPipeline pipeline;
    renderer.initialize_vulkan();
    
    float width = (float) renderer.get_swapchain_extent().width;
    float height = (float) renderer.get_swapchain_extent().height;
    
    // Initializing Models  =====================================================================================
    Game::Model room_model = Game::load_obj_model("./models/F1_2026.obj");
    room_model.model_matrix = glm::rotate(glm::mat4(1.f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    room_model.create_buffers(renderer);
    
    std::vector<Game::Model> models = {room_model};
    std::vector<std::string> texture_filenames = {"textures/Livery.jpg"};
    
    // Initializing UBO & Textures  =============================================================================
    float camera_d = 10.f;
    Game::UniformBufferObject ubo{};
    ubo.view = glm::lookAt(glm::vec3(camera_d, camera_d, camera_d), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
    ubo.proj = glm::perspective(glm::radians(45.f), width / height, 0.1f, 100.f);
    ubo.proj[1][1] *= -1;
    
    size_t ub_idx = renderer.create_uniform_group<Game::UniformBufferObject>(0, VK_SHADER_STAGE_VERTEX_BIT);
    renderer.update_uniform_group(ub_idx, &ubo);
    
    // renderer.add_texture("textures/viking_room.jpg", 1);
    renderer.add_texture_array(texture_filenames, 1024, 1024, 2, 1);
    
    // Initializing Program =====================================================================================
    std::vector<Engine::Pipeline*> pipelines = {&pipeline};
    renderer.initialize(pipelines);
    
    // Starting Game Loop   =====================================================================================
    int current_frame = 0;
    uint32_t image_index = 0;
    VkCommandBuffer command_buffer;

    using clock = std::chrono::high_resolution_clock;
    auto previous_frame_time = clock::now();
    float total_time = 0.0;

    while(renderer.begin_frame(current_frame, image_index, command_buffer)) {
        auto current_time = clock::now();
        float delta_time = std::chrono::duration<float>(current_time - previous_frame_time).count();
        previous_frame_time = current_time;
        total_time += delta_time;
        
        if (false) {
            double fps = delta_time > 0.0 ? 1.0 / delta_time: 0.0;
            fmt::println("{}", fps);
        }

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = renderer.get_render_pass();
        render_pass_info.framebuffer = renderer.get_framebuffer(image_index);
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = renderer.get_swapchain_extent();

        std::vector<VkClearValue> clear_colors(2);
        clear_colors[0].color = {{0.f, 0.f, 0.f, 1.f}};
        clear_colors[1].depthStencil = {1.f, 0};

        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_colors.size());
        render_pass_info.pClearValues = clear_colors.data();

        renderer.m_dispatch.cmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        renderer.m_dispatch.cmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_pipeline());

        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = static_cast<float>(renderer.get_swapchain_extent().width);
        viewport.height = static_cast<float>(renderer.get_swapchain_extent().height);
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        renderer.m_dispatch.cmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = renderer.get_swapchain_extent();
        renderer.m_dispatch.cmdSetScissor(command_buffer, 0, 1, &scissor);

        VkDescriptorSet cur_ds = renderer.get_descriptor_set(current_frame);
        renderer.m_dispatch.cmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_pipeline_layout(), 0, 1, &cur_ds, 0, nullptr);

        // Updating the uniform buffers ======================================================================
        ubo.view = glm::lookAt(glm::vec3(camera_d * cosf(total_time), camera_d * sinf(total_time),  camera_d), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
        renderer.update_uniform_group(ub_idx, &ubo);

        for(int mod = 0; mod < models.size(); mod++) {
            const Game::Model &model = models[mod];
            // Set push constants ============================================================================
            struct PC {
                glm::mat4 model;
                glm::vec4 idx;
            };

            PC pc{};
            pc.model = model.model_matrix;
            pc.idx = glm::vec4((float)mod, 0.0, 0.0, 0.0);

            renderer.m_dispatch.cmdPushConstants(command_buffer, pipeline.get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &pc);
            
            // Retreive vertex and index buffers
            VkBuffer vertex_buffers[] = {renderer.get_buffer(model.vertex_buffer_idx)};
            VkDeviceSize offsets[] = {0};
            VkBuffer index_buffer = renderer.get_buffer(model.index_buffer_idx);
            
            // Bind vertex and index buffers
            renderer.m_dispatch.cmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
            renderer.m_dispatch.cmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
            
            // draw call
            renderer.m_dispatch.cmdDrawIndexed(command_buffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
        }

        renderer.m_dispatch.cmdEndRenderPass(command_buffer);

        if(renderer.m_dispatch.endCommandBuffer(command_buffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffers!");

        renderer.end_frame();
    }

    renderer.cleanup();

    return 0;
}