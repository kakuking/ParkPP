#include <engine/models.h>
#include <game/default_pipeline.h>

namespace Game {

void DefaultPipeline::create_pipeline(Engine::Renderer &device, VkFormat image_format, VkRenderPass render_pass) {
    Engine::PipelineBuilder builder;
    std::vector<VkDescriptorSetLayout> layout = {device.get_descriptor_set_layout()};

    builder.add_push_constants(sizeof(Engine::PushConstants));

    builder.create_pipeline_layout(device, layout);
    
    builder.create_render_pass(device, device.get_swapchain(), render_pass);
    
    builder.set_shaders(device, "shaders/shader.vert.spv", "shaders/shader.frag.spv");
    builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
    builder.set_color_attachment_format(image_format);
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