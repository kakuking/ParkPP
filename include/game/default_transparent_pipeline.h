#pragma once

#include <engine/pipeline.h>
#include <engine/renderer.h>

namespace Game {

class DefaultTransparentPipeline: public Engine::Pipeline {
    void create_pipeline(Engine::Renderer &device, VkFormat image_format, VkRenderPass render_pass=VK_NULL_HANDLE) override;
};

}
