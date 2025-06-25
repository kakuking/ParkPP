#pragma once
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include <engine/shaders.h>
#include <engine/pipeline_builder.h>
// #include <engine/renderer.h>

namespace Engine {
class Renderer;

class Pipeline {
public:
    virtual void create_pipeline(Renderer &device, VkFormat image_format) = 0;
    virtual void destroy_pipeline(vkb::DispatchTable &dispatch_table) = 0;

    VkRenderPass get_render_pass() { return m_data.render_pass; }
    VkPipeline get_pipeline() { return m_data.pipeline; }
    VkPipelineLayout get_pipeline_layout() { return m_data.pipeline_layout; }

protected:
    PipelineData m_data;
};

}