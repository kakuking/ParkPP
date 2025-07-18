#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <engine/pipeline.h>
#include <engine/renderer.h>
#include <engine/models.h>

#include <game/default_pipeline.h>

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
    // Engine::Model room_model = Engine::load_obj_model("./models/F1_2026.obj", 0.f);
    Engine::Model car_model = Engine::load_obj_model_with_material("./models/F1_2026.obj", 0.f, "./models");
    Engine::Model plane_model = Engine::load_obj_model_with_material("./models/plane.obj", 4.f, "./models");
    car_model.create_buffers(renderer);
    plane_model.create_buffers(renderer);
    
    std::vector<Engine::Model> models = {car_model, plane_model};
    std::vector<std::string> texture_filenames = {"textures/Livery.jpg", "textures/Checkerboard.png", "textures/WheelCovers.jpg", "textures/TyreSoft.png", "textures/Checkerboard.png"};
    
    // Initializing UBO & Textures  =============================================================================
    float camera_d = 10.f;
    Game::UniformBufferObject ubo{};
    ubo.view = glm::lookAt(glm::vec3(camera_d, camera_d, camera_d), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
    ubo.proj = glm::perspective(glm::radians(45.f), width / height, 0.1f, 100.f);
    ubo.proj[1][1] *= -1;

    glm::vec3 light_pos = glm::vec3(0.0f, 5.0f, 5.0f); // light above and at an angle
    glm::vec3 light_target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 light_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 light_view = glm::lookAt(light_pos, light_target, light_up);

    
    float ortho_half_size = 6.0f; // gives you a 10x10 square area
    float near_plane = 1.0f;
    float far_plane = 10.0f;
    glm::mat4 light_proj = glm::ortho(
        -ortho_half_size, ortho_half_size,  // left, right
        -ortho_half_size, ortho_half_size,  // bottom, top
        near_plane, far_plane               // near, far
    );

    light_proj[1][1] *= -1;
    glm::mat4 light_pv = light_proj * light_view;

    renderer.add_light(light_pv, 0);

    light_pv;
    
    size_t ub_idx = renderer.create_uniform_group<Game::UniformBufferObject>(1, VK_SHADER_STAGE_VERTEX_BIT);
    renderer.update_uniform_group(ub_idx, &ubo);
    
    // renderer.add_texture("textures/viking_room.jpg", 1);
    uint32_t num_textures = static_cast<uint32_t>(texture_filenames.size());
    uint32_t layer_count = num_textures < 4 ? 4: num_textures;
    renderer.add_texture_array(texture_filenames, 1024, 1024, layer_count, 2);
    
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

        renderer.render_shadow_maps(command_buffer, models);
        // std::cout << "Begining next render pass\n";

        renderer.begin_render_pass(command_buffer, image_index);
        renderer.bind_pipeline_and_descriptors(command_buffer, 0, current_frame);
        renderer.set_default_viewport_and_scissor(command_buffer);

        // Updating the uniform buffers ======================================================================
        ubo.view = glm::lookAt(glm::vec3(camera_d * cosf(total_time), camera_d * sinf(total_time),  camera_d), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
        renderer.update_uniform_group(ub_idx, &ubo);

        for(int mod = 0; mod < models.size(); mod++) {
            struct PC {
                glm::mat4 light;
                glm::mat4 model;
            } pc;
            const Engine::Model &model = models[mod];

            // Retreive vertex and index buffers =============================================================
            VkBuffer vertex_buffers[] = {renderer.get_buffer(model.vertex_buffer_idx)};
            VkDeviceSize offsets[] = {0};
            VkBuffer index_buffer = renderer.get_buffer(model.index_buffer_idx);
            
            // Bind vertex and index buffers
            renderer.m_dispatch.cmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
            renderer.m_dispatch.cmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

            // Set push constants ============================================================================
            pc.light = light_pv;
            pc.model = model.model_matrix;
            renderer.m_dispatch.cmdPushConstants(command_buffer, pipeline.get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);
            
            // draw call
            renderer.m_dispatch.cmdDrawIndexed(command_buffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
        }

        renderer.end_render_pass_and_command_buffer(command_buffer);
        renderer.end_frame();
    }

    renderer.cleanup();

    return 0;
}