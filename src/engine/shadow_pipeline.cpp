#include <engine/pipeline.h>
#include <engine/models.h>

namespace Engine {

void ShadowPipeline::create_pipeline(Engine::Renderer &device, VkFormat image_format, VkRenderPass old_render_pass) {
    // to remove warning
    (void)image_format;
    (void)old_render_pass;

    Engine::PipelineBuilder builder;
    std::vector<VkDescriptorSetLayout> layout = {};

    struct LightModel {
        glm::mat4 mvp;
        glm::mat4 model;
    };

    builder.add_push_constants(sizeof(LightModel));
    builder.disable_msaa();
    builder.disable_color_attachment();
    builder.disable_dynamic_state();

    builder.create_pipeline_layout(device, layout);
    
    builder.create_shadow_render_pass(device);
    
    builder.set_shaders(device, "shaders/shadow_shader.vert.spv", "shaders/shadow_shader.frag.spv");
    builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
    builder.enable_culling(VK_CULL_MODE_BACK_BIT);
    builder.disable_blending();
    builder.enable_depth_test();
    builder.enable_depth_write();

    auto bind_desc = Engine::Vertex::get_binding_description();
    auto attr_desc = Engine::Vertex::get_attribute_description();

    builder.set_vertex_binding_and_attrs(bind_desc, attr_desc);

    m_data = builder.build(device);
}
}