#include <iostream>
#include <glm/glm.hpp>
#include <VkBootstrap.h>
#include <fmt/format.h>

int main() {
    glm::vec2 pog(0.0, 1.0);
    vkb::InstanceBuilder builder;

    fmt::println("pog - ({}, {})", pog.x, pog.y);

    return 0;
}