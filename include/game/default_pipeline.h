#pragma once

#include <engine/pipeline.h>
#include <engine/renderer.h>

namespace Game {

class DefaultPipeline: public Engine::Pipeline {
    void create_pipeline(Engine::Renderer &device, VkFormat image_format) override;
    void destroy_pipeline(vkb::DispatchTable &dispatch_table) override;
};

}
