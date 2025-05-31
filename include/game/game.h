#include <glm/glm.hpp>
#include <engine/renderer.h>

namespace Game {
class GameCore {
public:
    virtual ~GameCore() {}
    virtual void initialize_game() = 0;
    virtual void loop() = 0;
private:
    void add_mesh();
    void set_camera();
};
}