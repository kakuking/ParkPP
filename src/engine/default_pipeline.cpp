// #include <engine/pipeline.h>
// #include <engine/graphics_device.h>
// #include <engine/models.h>
// #include <iostream>

// namespace Engine {
// class DefaultPipeline: Pipeline {
//     void create_pipeline(GraphicsDevice &device, VkFormat image_format) override {
//         PipelineBuilder builder;

//         std::vector<VkDescriptorSetLayout> layout = {device.get_descriptor_set_layout()};

//         builder.create_pipeline_layout(device, layout.data());
//         builder.set_shaders(device, "shaders/shader.frag.spv", "shaders.vert.spv");
//         builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
//         builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
//         builder.set_color_attachment_format(image_format);
//         builder.enable_culling(VK_CULL_MODE_BACK_BIT);
//         builder.disable_blending();
//         builder.disable_depth_test();

//         auto bind_desc = Vertex::get_binding_description();
//         auto attr_desc = Vertex::get_attribute_description();

//         builder.set_vertex_binding_and_attrs(bind_desc, attr_desc);

//         m_data = builder.build(device);
//     }

//     void destroy_pipeline(vkb::DispatchTable &dispatch_table) override {
//         m_data.frag_shader.destroy_shader(dispatch_table);
//         m_data.vert_shader.destroy_shader(dispatch_table);
//         dispatch_table.destroyRenderPass(m_data.render_pass, nullptr);
//         dispatch_table.destroyPipelineLayout(m_data.pipeline_layout, nullptr);
//         dispatch_table.destroyPipeline(m_data.pipeline, nullptr);
//     }
// };
// }