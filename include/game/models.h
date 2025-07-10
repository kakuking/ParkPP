#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <array>
#include <unordered_map>

#include <tiny_obj_loader.h>

#include <engine/renderer.h>

namespace Game {
struct Vertex {
    glm::vec3 pos;
    float u;
    glm::vec3 color;
    float v;

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

        std::vector<VkVertexInputAttributeDescription> attr_desc(4);
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

        return attr_desc;
    }
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Model {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    size_t vertex_buffer_idx, index_buffer_idx;
    size_t vertex_buffer_size, index_buffer_size;

    void create_buffers(Engine::Renderer &renderer);
    void refresh_buffers(Engine::Renderer &renderer);
};

Model load_obj_model(std::string filename);

}

namespace std {
    template<> struct hash<Game::Vertex> {
        size_t operator()(Game::Vertex const& vertex) const {
            size_t h1 = hash<glm::vec3>()(vertex.pos);
            size_t h2 = hash<glm::vec3>()(vertex.color);
            size_t h3 = hash<float>()(vertex.u);
            size_t h4 = hash<float>()(vertex.v);

            return (((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1)) ^ (h4 << 1);
        }
    };
}