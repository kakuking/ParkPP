#include <engine/renderer.h> 

int main() {    
    Engine::Renderer renderer;

    try {
        renderer.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}