#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <array>
#include <unordered_map>

#include <tiny_obj_loader.h>

#include <engine/renderer.h>

namespace Engine {
struct Vertex {
    glm::vec3 pos;
    float u;
    glm::vec3 color;
    float v;
    glm::vec3 normal;
    float material_idx;

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && u == other.u && v == other.v;
    }

    static VkVertexInputBindingDescription get_binding_description() {

        VkVertexInputBindingDescription binding_desc{};
        binding_desc.binding = 0;
        binding_desc.stride = sizeof(Vertex);
        binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_desc;
    }

    static std::vector<VkVertexInputAttributeDescription> get_attribute_description() {

        std::vector<VkVertexInputAttributeDescription> attr_desc(6);
        
        attr_desc[0].binding = 0;
        attr_desc[0].location = 0;
        attr_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr_desc[0].offset = offsetof(Vertex, pos);

        attr_desc[1].binding = 0;
        attr_desc[1].location = 1;
        attr_desc[1].format = VK_FORMAT_R32_SFLOAT;
        attr_desc[1].offset = offsetof(Vertex, u);
        
        attr_desc[2].binding = 0;
        attr_desc[2].location = 2;
        attr_desc[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr_desc[2].offset = offsetof(Vertex, color);
        
        attr_desc[3].binding = 0;
        attr_desc[3].location = 3;
        attr_desc[3].format = VK_FORMAT_R32_SFLOAT;
        attr_desc[3].offset = offsetof(Vertex, v);

        attr_desc[4].binding = 0;
        attr_desc[4].location = 4;
        attr_desc[4].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr_desc[4].offset = offsetof(Vertex, normal);
        
        attr_desc[5].binding = 0;
        attr_desc[5].location = 5;
        attr_desc[5].format = VK_FORMAT_R32_SFLOAT;
        attr_desc[5].offset = offsetof(Vertex, material_idx);

        return attr_desc;
    }
};

struct Model {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    size_t vertex_buffer_idx, index_buffer_idx;
    size_t vertex_buffer_size, index_buffer_size;

    glm::mat4 model_matrix = glm::mat4(1.f);
    float base_texture=0;   // idx of first texture (float so I can pass it as an attr)

    void create_buffers(Engine::Renderer &renderer);
    void refresh_buffers(Engine::Renderer &renderer);
};

Model load_obj_model(std::string filename, float base_texture=0.f);
Model load_obj_model_with_material(std::string filename, float base_texture=0.f, std::string base_dir="");

}

namespace std {
    template<> struct hash<Engine::Vertex> {
        size_t operator()(Engine::Vertex const& vertex) const {
            size_t h1 = hash<glm::vec3>()(vertex.pos);
            size_t h2 = hash<glm::vec3>()(vertex.color);
            size_t h3 = hash<float>()(vertex.u);
            size_t h4 = hash<float>()(vertex.v);

            return (((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1)) ^ (h4 << 1);
        }
    };
}

namespace Game {
struct UniformBufferObject {
    // glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
}