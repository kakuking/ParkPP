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
    // Game::Model room_model = Game::load_obj_model("./models/F1_2026.obj", 0.f);
    Game::Model room_model = Game::load_obj_model_with_material("./models/F1_2026.obj", 0.f, "./models");
    room_model.model_matrix = glm::rotate(glm::mat4(1.f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    room_model.create_buffers(renderer);
    
    std::vector<Game::Model> models = {room_model};
    std::vector<std::string> texture_filenames = {"textures/Livery.jpg", "textures/Checkerboard.png", "textures/WheelCovers.jpg", "textures/TyreSoft.png"};
    
    // Initializing UBO & Textures  =============================================================================
    float camera_d = 10.f;
    Game::UniformBufferObject ubo{};
    ubo.view = glm::lookAt(glm::vec3(camera_d, camera_d, camera_d), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
    ubo.proj = glm::perspective(glm::radians(45.f), width / height, 0.1f, 100.f);
    ubo.proj[1][1] *= -1;
    
    size_t ub_idx = renderer.create_uniform_group<Game::UniformBufferObject>(0, VK_SHADER_STAGE_VERTEX_BIT);
    renderer.update_uniform_group(ub_idx, &ubo);
    
    // renderer.add_texture("textures/viking_room.jpg", 1);
    renderer.add_texture_array(texture_filenames, 1024, 1024, 4, 1);
    
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

        renderer.begin_render_pass(command_buffer, image_index);
        renderer.bind_pipeline_and_descriptors(command_buffer, 0, current_frame);
        renderer.set_default_viewport_and_scissor(command_buffer);

        // Updating the uniform buffers ======================================================================
        ubo.view = glm::lookAt(glm::vec3(camera_d * cosf(total_time), camera_d * sinf(total_time),  camera_d), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
        renderer.update_uniform_group(ub_idx, &ubo);

        for(int mod = 0; mod < models.size(); mod++) {
            const Game::Model &model = models[mod];

            // Retreive vertex and index buffers =============================================================
            VkBuffer vertex_buffers[] = {renderer.get_buffer(model.vertex_buffer_idx)};
            VkDeviceSize offsets[] = {0};
            VkBuffer index_buffer = renderer.get_buffer(model.index_buffer_idx);
            
            // Bind vertex and index buffers
            renderer.m_dispatch.cmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
            renderer.m_dispatch.cmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

            // Set push constants ============================================================================
            renderer.m_dispatch.cmdPushConstants(command_buffer, pipeline.get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model.model_matrix);
            
            // draw call
            renderer.m_dispatch.cmdDrawIndexed(command_buffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
        }

        renderer.end_render_pass(command_buffer);
        renderer.end_frame();
    }

    renderer.cleanup();

    return 0;
}