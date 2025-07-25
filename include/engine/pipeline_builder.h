#pragma once
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include <engine/shaders.h>

namespace Engine {
class Renderer;
    
struct PipelineData {
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    Shader vert_shader, frag_shader;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    bool m_unique_render_pass = true;
};

class PipelineBuilder {
public:
    void create_pipeline_layout(Renderer &device, std::vector<VkDescriptorSetLayout> descriptor_set_layout);

    void set_shaders(Renderer &device, const std::string &vert_shader_filename, const std::string &frag_shader_filename);
    void add_push_constants(uint32_t pc_size, uint32_t offset=0, VkShaderStageFlags shader_stage=VK_SHADER_STAGE_VERTEX_BIT);
    
    void set_input_topology(VkPrimitiveTopology input_topology) { m_input_topology = input_topology; }
    void set_polygon_mode(VkPolygonMode polygon_mode) { m_polygon_mode = polygon_mode; }

    void set_color_attachment_format(VkFormat draw_format) { m_draw_format = draw_format; }
    void set_depth_attachment_format(VkFormat depth_format) { m_depth_format = depth_format; }

    void enable_culling(VkCullModeFlags cull_mode) { m_cull_mode = cull_mode; }
    void disable_culling() { m_cull_mode = VK_CULL_MODE_NONE; }

    void enable_blending() { m_enable_blending = true; }
    void disable_blending() { m_enable_blending = false; }

    void enable_depth_test() { m_enable_depth_test = true; }
    void disable_depth_test() { m_enable_depth_test = false; }

    void enable_depth_write() { m_enable_depth_write = true; }
    void disable_depth_write() { m_enable_depth_write = false; }

    void enable_msaa() { m_enable_msaa = true; }
    void disable_msaa() { m_enable_msaa = false; }

    void enable_color_attachment() { m_use_color_attachment = true; }
    void disable_color_attachment() { m_use_color_attachment = false; }

    void enable_dynamic_state() { m_enable_dynamic_state = true; }
    void disable_dynamic_state() { m_enable_dynamic_state = false; }

    void set_vertex_binding_and_attrs(VkVertexInputBindingDescription vertex_binding_desc, std::vector<VkVertexInputAttributeDescription> vertex_input_attr_desc) { m_vertex_binding_desc = vertex_binding_desc; m_vertex_input_attr_desc = vertex_input_attr_desc; }
    
    PipelineData build(Renderer &device);

    void create_render_pass(Renderer &renderer, vkb::Swapchain swapchain, VkRenderPass old_render_pass);
    void set_render_pass(VkRenderPass render_pass) { m_render_pass = render_pass; }
    void create_shadow_render_pass(Renderer &renderer); 

private:
    VkPipelineDynamicStateCreateInfo get_dynamic_state_create_info();
    VkPipelineVertexInputStateCreateInfo get_vertex_input_state_create_info();
    VkPipelineInputAssemblyStateCreateInfo get_input_assembly_state_create_info();
    VkViewport get_viewport(float width, float height);
    VkRect2D get_scissor(uint32_t width, uint32_t height);
    VkPipelineViewportStateCreateInfo get_viewport_state(VkExtent2D extent);
    VkPipelineRasterizationStateCreateInfo get_rasterizer_state();
    VkPipelineMultisampleStateCreateInfo get_multisampling(VkSampleCountFlagBits num_samples);
    VkPipelineColorBlendAttachmentState get_color_blend_attachment();
    VkPipelineColorBlendStateCreateInfo get_color_blend_state(VkPipelineColorBlendAttachmentState &attachment);
    VkGraphicsPipelineCreateInfo get_pipeline_create_info();
    VkPipelineDepthStencilStateCreateInfo get_depth_stencil_create_info();
    void create_descriptor_set_layout(vkb::DispatchTable &dispatch);

    Shader m_vert_shader, m_frag_shader;
    VkRenderPass m_render_pass = VK_NULL_HANDLE;;
    VkPipeline m_pipeline;
    VkPipelineLayout m_pipeline_layout;
    std::vector<VkDynamicState> m_dynamic_states;
    std::vector<VkPushConstantRange> m_push_constant_ranges;
    
    VkPrimitiveTopology m_input_topology;
    VkPolygonMode m_polygon_mode;
    VkFormat m_draw_format;
    VkFormat m_depth_format;
    VkCullModeFlags m_cull_mode = VK_CULL_MODE_NONE;
    VkViewport m_viewport;
    VkRect2D m_scissor;

    bool m_unique_render_pass = true;
    
    bool m_enable_blending = false;
    bool m_enable_depth_test = false;
    bool m_enable_depth_write = false;
    bool m_enable_msaa = true;
    bool m_use_color_attachment = true;
    bool m_enable_dynamic_state = true;
    VkVertexInputBindingDescription m_vertex_binding_desc; std::vector<VkVertexInputAttributeDescription> m_vertex_input_attr_desc;
};

}