#include <iostream>
#include <glm/glm.hpp>
#include <VkBootstrap.h>
#include <fmt/format.h>

#include <engine.hpp>

void EngineCore::hello_world() {
    std::cout << "Hello world -- " << ping << " -- \n";
}

int main() {
    glm::vec2 pog(0.0, 1.0);
    vkb::InstanceBuilder builder;
    EngineCore eng;
    
    eng.hello_world();
    fmt::println("pog - ({}, {})", pog.x, pog.y);

    return 0;
}