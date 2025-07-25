#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <engine/pipeline.h>
#include <engine/renderer.h>
#include <engine/models.h>
#include <engine/scene.h>

#include <game/default_pipeline.h>
#include <game/default_transparent_pipeline.h>

#include <fmt/format.h>
#include <iostream>

#include <chrono>   // for FPS

#include <filesystem>   // for abs filename

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>

    void attach_console() {
        if (AttachConsole(ATTACH_PARENT_PROCESS)) {
            // Redirect stdio handles to console
            FILE* dummy;
            freopen_s(&dummy, "CONOUT$", "w", stdout);
            freopen_s(&dummy, "CONOUT$", "w", stderr);
            freopen_s(&dummy, "CONIN$", "r", stdin);

            // Optional: disable buffering to see output immediately
            setvbuf(stdout, nullptr, _IONBF, 0);
            setvbuf(stderr, nullptr, _IONBF, 0);
        }
    }
#endif

int main(int argc, char** argv) {
    (void) argc; (void) argv;

    #ifdef _WIN32
        attach_console(); // Attach to console of parent process if any
    #endif

    // Initializing Vulkan  ============================================================================
    Engine::Renderer renderer;
    Game::DefaultPipeline pipeline;
    Game::DefaultTransparentPipeline transparent_pipeline;
    renderer.initialize_vulkan();
    
    float width = (float) renderer.get_swapchain_extent().width;
    float height = (float) renderer.get_swapchain_extent().height;
    
    // Initializing Scene  =============================================================================
    float aspect_ratio = width/height;
    Engine::Scene scene(aspect_ratio);

    std::filesystem::path scene_path;
    if (argc > 1) {
        scene_path = std::filesystem::absolute(argv[1]);
    } else {
        scene_path = std::filesystem::absolute("./scenes/scene.xml");
    }

    scene.load_scene_from_xml(scene_path.string());

    /*
    Engine::ModelInfo car = scene.add_model("./models/F1_2026.glb", {"textures/Livery.jpg", "textures/Checkerboard.png", "textures/WheelCovers.jpg", "textures/TyreSoft.png"});
    Engine::ModelInfo ground = scene.add_model("./models/plane.obj", {"textures/Checkerboard.png"});
    Engine::ModelInfo red_plane = scene.add_model("./models/plane.obj", {"textures/red_board.png"}, false);

    glm::mat4 t = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 4.f)) * glm::scale(glm::mat4(1.f), glm::vec3(.5f, .5f, 1.f));
    scene.update_transparent_model_transform(red_plane, t, false);

    float camera_d = 10.f;
    glm::vec3 eye(camera_d, camera_d, camera_d);
    glm::vec3 look_at(0.f, 0.f, 0.f);
    glm::vec3 up(0.f, 0.f, 1.f);

    glm::vec3 light_pos = glm::vec3(0.0f, 5.0f, 3.0f); // light above and at an angle
    glm::vec3 light_target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 light_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 light_color(1.0, 1.0, 1.0);

    scene.set_perspective_camera(eye, look_at, up, width / height, 0.1f, 100.f, 45.f);
    scene.add_orthographic_light(light_color, light_pos, light_target, light_up, 0.1f, 15.f, 6.f);
    */

    scene.create_buffers(renderer);
    
    // Initializing Program ============================================================================
    std::vector<Engine::Pipeline*> pipelines = {&pipeline, &transparent_pipeline};
    renderer.initialize(pipelines);
    
    // Starting Game Loop   ============================================================================
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

        // Updating scene ==============================================================================
        width = (float) renderer.get_swapchain_extent().width;
        height = (float) renderer.get_swapchain_extent().height;
        scene.update(delta_time, width / height);

        renderer.render_shadow_maps(command_buffer, scene.m_opaque_models);

        renderer.begin_render_pass(command_buffer, image_index);
        // Rendering opaque objects ====================================================================
        renderer.bind_pipeline_and_descriptors(command_buffer, 0, current_frame);
        renderer.set_default_viewport_and_scissor(command_buffer);

        scene.render_opaque_models(renderer, command_buffer);

        // Rendering transparent objects ===============================================================
        if (scene.m_transparent_models.size() > 0) {
            renderer.bind_pipeline_and_descriptors(command_buffer, 1, current_frame);
            renderer.set_default_viewport_and_scissor(command_buffer);
    
            scene.render_transparent_models(renderer, command_buffer);
        }

        renderer.end_render_pass_and_command_buffer(command_buffer);
        renderer.end_frame();
    }

    renderer.cleanup();

    return 0;
}