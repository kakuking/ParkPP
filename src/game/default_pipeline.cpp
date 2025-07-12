#include <game/default_pipeline.h>
#include <game/models.h>

namespace Game {

void DefaultPipeline::create_pipeline(Engine::Renderer &device, VkFormat image_format) {
    Engine::PipelineBuilder builder;
    std::vector<VkDescriptorSetLayout> layout = {device.get_descriptor_set_layout()};

    builder.add_push_constants(sizeof(glm::mat4) + sizeof(glm::vec4));

    builder.create_pipeline_layout(device, layout.data());
    
    builder.create_render_pass(device, device.get_swapchain());
    
    builder.set_shaders(device, "shaders/shader.vert.spv", "shaders/shader.frag.spv");
    builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
    builder.set_color_attachment_format(image_format);
    builder.enable_culling(VK_CULL_MODE_BACK_BIT);
    builder.disable_blending();
    builder.disable_depth_test();

    auto bind_desc = Game::Vertex::get_binding_description();
    auto attr_desc = Game::Vertex::get_attribute_description();

    builder.set_vertex_binding_and_attrs(bind_desc, attr_desc);

    m_data = builder.build(device);
}

void DefaultPipeline::destroy_pipeline(vkb::DispatchTable &dispatch_table) {
    m_data.frag_shader.destroy_shader(dispatch_table);
    m_data.vert_shader.destroy_shader(dispatch_table);
    dispatch_table.destroyRenderPass(m_data.render_pass, nullptr);
    dispatch_table.destroyPipelineLayout(m_data.pipeline_layout, nullptr);
    dispatch_table.destroyPipeline(m_data.pipeline, nullptr);
}
}