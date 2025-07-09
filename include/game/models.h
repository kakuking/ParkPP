#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <array>

#include <tiny_obj_loader.h>

namespace Game {
struct Vertex {
    glm::vec3 pos;
    float u;
    glm::vec3 color;
    float v;

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

const std::vector<Vertex> vertices = {
    // Base square (z = 0)
    {{-0.5f, -0.5f, 0.0f},  1.f,    {1.f, 0.f, 0.f},    0.f}, // 0
    {{0.5f, -0.5f, 0.0f},   0.f,    {0.f, 1.f, 0.f},    0.f}, // 1
    {{0.5f,  0.5f, 0.0f},   0.f,    {0.f, 0.f, 1.f},    1.f}, // 2
    {{-0.5f,  0.5f, 0.0f},  1.f,    {1.f, 1.f, 1.f},    1.f}, // 3

    // Apex vertex (z = 0.5)
    {{0.0f, 0.0f, 0.5f},    .5f,    {1.f, 1.f, 0.f},    .5f}, // 4
};

const std::vector<uint16_t> indices = {
    // plane
    // 0, 1, 2,
    // 2, 3, 0,
    

    // Base (2 triangles)
    2, 1, 0,
    0, 3, 2,
    
    // Side faces (each triangle connects to apex)
    0, 1, 4, // front
    1, 2, 4, // right
    2, 3, 4, // back
    3, 0, 4, // left
};

}