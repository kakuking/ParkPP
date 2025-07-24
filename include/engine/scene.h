#pragma once

#include <engine/models.h>
#include <pugixml.hpp>

const int MAX_VERTICES_IN_BUFFER = 4194304; // 2^21

namespace Engine {

class Scene {
public:
    Scene(float ar): m_aspect_ratio(ar) {}

    void load_scene_from_xml(std::string filename);

    void render_opaque_models(Renderer &renderer, VkCommandBuffer &command_buffer);
    void render_transparent_models(Renderer &renderer, VkCommandBuffer &command_buffer);

    void create_buffers(Renderer &renderer);

    void add_light(glm::vec3 light_color, glm::vec3 light_pos, glm::mat4 light_matrix);
    void set_camera(glm::mat4 proj, glm::mat4 view);

    void set_perspective_camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up, float forced_aspect_ratio=0.0f, float near_plane=0.1f, float far_plane=10.f, float fov_degrees=45.f);
    void set_orthographic_camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up, float near_plane=0.1f, float far_plane=10.f);

    void add_orthographic_light(glm::vec3 color, glm::vec3 position, glm::vec3 look_at, glm::vec3 up, float near_plane, float far_plane, float ortho_half_size);

    ModelInfo add_model(std::string filename, std::vector<std::string> texture_filename, bool opaque=true, bool updating=false);

    void update_opaque_model_transform(ModelInfo &mi, glm::mat4 transform, bool replace=false);
    void update_transparent_model_transform(ModelInfo &mi, glm::mat4 transform, bool replace=false);

    void update_camera_view(glm::mat4 view) { m_push_constants.view = view; }
    void update_camera_proj(glm::mat4 proj) { m_push_constants.proj = proj; }

    int num_textures() { return (int)m_textures.size(); }

    void update(float delta_time, float aspect_ratio);

    std::vector<Engine::Model> m_opaque_models;
    std::vector<Engine::Model> m_transparent_models;
private:
    float get_or_add_texture(std::string texture_filename);
    void load_obj_model(Engine::Model &output_model, std::string filename, float base_texture=0.f);
    void load_obj_model_with_material(Engine::Model &output_model, std::string filename, float base_texture=0.f, std::string base_dir="");
    
    ModelInfo add_obj_model(std::string filename, std::vector<std::string> texture_filename, bool opaque=true, bool updating=false);
    ModelInfo add_gltf_model(std::string filename, std::vector<std::string> texture_filename, bool opaque=true, bool updating=false);

    // helpers for XML parse
    std::vector<float> parse_floats(const std::string& str);
    std::vector<std::string> parse_strings(const std::string& str);

    void process_node(const pugi::xml_node& node);
    void process_camera(const pugi::xml_node& node);
    void process_light(const pugi::xml_node& node);
    void process_transform(const pugi::xml_node& node, glm::mat4 &out);
    void process_mesh(const pugi::xml_node& node);

    size_t m_last_non_updating_opaque_model;
    size_t m_last_non_updating_transparent_model;
    
    std::vector<glm::mat4> m_model_transform_matrices;
    std::vector<Light> m_lights;
    std::vector<std::string> m_textures;
    PushConstants m_push_constants;

    bool perspective = true;
    float m_aspect_ratio;
    float m_fov, m_near_plane, m_far_plane;

    float total_time = 0.f;
};
}