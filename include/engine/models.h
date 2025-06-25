#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

#include <array>

namespace Engine {
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription get_binding_description() {

        VkVertexInputBindingDescription binding_desc{};
        binding_desc.binding = 0;
        binding_desc.stride = sizeof(Vertex);
        binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_desc;
    }

    static std::vector<VkVertexInputAttributeDescription> get_attribute_description() {

        std::vector<VkVertexInputAttributeDescription> attr_desc(2);
        attr_desc[0].binding = 0;
        attr_desc[0].location = 0;
        attr_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr_desc[0].offset = offsetof(Vertex, pos);

        attr_desc[1].binding = 0;
        attr_desc[1].location = 1;
        attr_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr_desc[1].offset = offsetof(Vertex, color);

        return attr_desc;
    }
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.f, 0.f, 0.f}},
    {{0.5f, -0.5f, 0.0f}, {0.f, 1.f, 0.f}},
    {{0.5f, 0.5f, 0.0f}, {0.f, 0.f, 1.f}},
    {{-0.5f, 0.5f, 0.0f}, {1.f, 1.f, 1.f}},
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

}