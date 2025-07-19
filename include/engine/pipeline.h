#pragma once
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include <engine/pipeline_builder.h>

namespace Engine {
class Renderer;

class Pipeline {
public:
    virtual void create_pipeline(Renderer &device, VkFormat image_format, VkRenderPass render_pass=VK_NULL_HANDLE) = 0;

    void destroy_pipeline(vkb::DispatchTable &dispatch_table) {
        m_data.frag_shader.destroy_shader(dispatch_table);
        m_data.vert_shader.destroy_shader(dispatch_table);
        if (m_data.m_unique_render_pass)
            dispatch_table.destroyRenderPass(m_data.render_pass, nullptr);
        dispatch_table.destroyPipelineLayout(m_data.pipeline_layout, nullptr);
        dispatch_table.destroyPipeline(m_data.pipeline, nullptr);
    }

    VkRenderPass get_render_pass() { return m_data.render_pass; }
    VkPipeline get_pipeline() { return m_data.pipeline; }
    VkPipelineLayout get_pipeline_layout() { return m_data.pipeline_layout; }

protected:
    PipelineData m_data;
};

class ShadowPipeline: public Pipeline {
    void create_pipeline(Engine::Renderer &device, VkFormat image_format, VkRenderPass render_pass=VK_NULL_HANDLE) override;
};

}