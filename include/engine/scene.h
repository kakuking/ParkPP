#pragma once

#include <engine/models.h>

const int MAX_VERTICES_IN_BUFFER = 4194304; // 2^21

namespace Engine {

class Scene {
public:

    void render_opaque_models(Renderer &renderer, VkCommandBuffer &command_buffer);
    void render_transparent_models(Renderer &renderer, VkCommandBuffer &command_buffer);

    void create_buffers(Renderer &renderer);

    int add_light(Renderer &renderer, glm::vec3 light_pos, glm::mat4 light_matrix);
    void set_camera(glm::mat4 proj, glm::mat4 view);

    void set_perspective_camera(glm::vec3 eye, glm::vec3 look_at, glm::vec3 up, float aspect_ratio, float near_plane=0.1f, float far_plane=10.f, float fov_degrees=45.f);
    void set_orthographic_camera(glm::vec3 eye, glm::vec3 look_at, glm::vec3 up, float near_plane=0.1f, float far_plane=10.f);

    int add_orthographic_light(Renderer &renderer, glm::vec3 position, glm::vec3 look_at, glm::vec3 up, float near_plane=0.1f, float far_plane=10.f, float ortho_half_size=6.f);

    ModelInfo add_model(std::string filename, std::vector<std::string> texture_filename, bool opaque=true, bool updating=false);
    ModelInfo add_model_with_material(std::string filename, std::vector<std::string> texture_filename, bool opaque=true, bool updating=false);

    void update_opaque_model_transform(ModelInfo &mi, glm::mat4 transform, bool replace=false);
    void update_transparent_model_transform(ModelInfo &mi, glm::mat4 transform, bool replace=false);

    void update_camera_view(glm::mat4 view) { m_push_constants.view = view; }
    void update_camera_proj(glm::mat4 proj) { m_push_constants.proj = proj; }

    int num_textures() { return (int)m_textures.size(); }

    void update(float delta_time);

    std::vector<Engine::Model> m_opaque_models;
    std::vector<Engine::Model> m_transparent_models;
private:
    float get_or_add_texture(std::string texture_filename);
    void load_obj_model(Engine::Model &output_model, std::string filename, float base_texture=0.f);
    void load_obj_model_with_material(Engine::Model &output_model, std::string filename, float base_texture=0.f, std::string base_dir="");


    size_t m_last_non_updating_opaque_model;
    size_t m_last_non_updating_transparent_model;
    
    std::vector<glm::mat4> m_model_transform_matrices;
    std::vector<int> m_lights;
    std::vector<std::string> m_textures;
    PushConstants m_push_constants;

    float total_time = 0.f;
};
}