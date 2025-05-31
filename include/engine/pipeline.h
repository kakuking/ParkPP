#pragma once
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include <engine/shaders.h>
#include <engine/models.h>

namespace Engine {

class Pipeline {
public:
    void create_pipeline(vkb::DispatchTable &dispatch, vkb::Swapchain swapchain, const std::string &vert_shader_filename, const std::string &frag_shader_filename);
    void destroy_pipeline(vkb::DispatchTable &dispatch);

    VkRenderPass get_render_pass();
    VkPipeline get_pipeline();
    VkPipelineLayout get_pipeline_layout();
    VkDescriptorSetLayout get_descriptor_set_layout();

private:

    VkPipelineDynamicStateCreateInfo get_dynamic_state_create_info();
    VkPipelineVertexInputStateCreateInfo get_vertex_input_state_create_info();
    VkPipelineInputAssemblyStateCreateInfo get_input_assembly_state_create_info();
    VkViewport get_viewport(const vkb::Swapchain &swapchain);
    VkRect2D get_scissor(const vkb::Swapchain &swapchain);
    VkPipelineViewportStateCreateInfo get_viewport_state();
    VkPipelineRasterizationStateCreateInfo get_rasterizer_state();
    VkPipelineMultisampleStateCreateInfo get_multisampling();
    VkPipelineColorBlendAttachmentState get_color_blend_attachment();
    VkPipelineColorBlendStateCreateInfo get_color_blend_state(VkPipelineColorBlendAttachmentState &attachment);
    VkPipelineLayoutCreateInfo get_pipeline_layout_create_info();
    VkGraphicsPipelineCreateInfo get_pipeline_create_info();
    void create_render_pass(vkb::DispatchTable &dispatch, vkb::Swapchain swapchain);
    void create_descriptor_set_layout(vkb::DispatchTable &dispatch);

    Shader m_vert_shader, m_frag_shader;
    VkDescriptorSetLayout m_descriptor_set_layout;
    VkRenderPass m_render_pass = VK_NULL_HANDLE;;
    VkPipelineLayout m_pipeline_layout;
    VkPipeline m_pipeline;
    std::vector<VkDynamicState> m_dynamic_states;
};

}