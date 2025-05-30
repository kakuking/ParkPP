#include <engine/pipeline.h>
#include <iostream>

namespace Engine {

void Pipeline::create_pipeline(vkb::DispatchTable &dispatch, vkb::Swapchain swapchain, const std::string &vert_shader_filename, const std::string &frag_shader_filename) {
    // std::cout << "Creating render pass\n";
    create_render_pass(dispatch, swapchain);
    
    // std::cout << "Creating vertex shader\n";
    m_vert_shader.create_shader(dispatch, vert_shader_filename, VK_SHADER_STAGE_VERTEX_BIT);
    // std::cout << "Creating frag shader\n";
    m_frag_shader.create_shader(dispatch, frag_shader_filename, VK_SHADER_STAGE_FRAGMENT_BIT);

    // std::cout << "Creating shader stages\n";
    VkPipelineShaderStageCreateInfo shader_stages[] = {
        m_vert_shader.get_shader_stage_create_info(),
        m_frag_shader.get_shader_stage_create_info()
    };

    // std::cout << "Creating dyn states\n";
    VkPipelineDynamicStateCreateInfo dynamic_states = get_dynamic_state_create_info();
    // std::cout << "Creating vert inp info\n";
    VkPipelineVertexInputStateCreateInfo vertex_input_info = get_vertex_input_state_create_info();
    // std::cout << "Creating inp assembly\n";
    VkPipelineInputAssemblyStateCreateInfo input_assembly = get_input_assembly_state_create_info();
    // VkViewport viewport = get_viewport(swapchain);
    // VkRect2D scissor = get_scissor(swapchain);
    // std::cout << "Creating viewport state\n";
    VkPipelineViewportStateCreateInfo viewport_state = get_viewport_state();
    // std::cout << "Creating rast\n";
    VkPipelineRasterizationStateCreateInfo rasterizer = get_rasterizer_state();
    // std::cout << "Creating ms\n";
    VkPipelineMultisampleStateCreateInfo multisampling = get_multisampling();
    // std::cout << "Creating cba\n";
    VkPipelineColorBlendAttachmentState color_blend_attachment = get_color_blend_attachment();
    // std::cout << "Creating cb\n";
    VkPipelineColorBlendStateCreateInfo color_blending = get_color_blend_state(color_blend_attachment);
    // std::cout << "Creating layout\n";
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = get_pipeline_layout_create_info();
    
    if(dispatch.createPipelineLayout(&pipeline_layout_create_info, nullptr, &m_pipeline_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create a pipline layout!");

    // std::cout << "Creating pci\n";
    VkGraphicsPipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.stageCount = 2;
    create_info.pStages = shader_stages;
    create_info.pVertexInputState = &vertex_input_info;
    create_info.pInputAssemblyState = &input_assembly;
    create_info.pViewportState = &viewport_state;
    create_info.pRasterizationState = &rasterizer;
    create_info.pMultisampleState = &multisampling;
    create_info.pDepthStencilState = nullptr;
    create_info.pColorBlendState = &color_blending;
    create_info.pDynamicState = &dynamic_states;
    create_info.layout = m_pipeline_layout;
    create_info.renderPass = m_render_pass;
    create_info.subpass = 0;
    create_info.basePipelineHandle = VK_NULL_HANDLE;
    create_info.basePipelineIndex = -1;

    // std::cout << "Creating pipeline\n";
    if(dispatch.createGraphicsPipelines(VK_NULL_HANDLE, 1, &create_info, nullptr, &m_pipeline) != VK_SUCCESS)
        throw std::runtime_error("Could not create graphics pipeline!");

}

void Pipeline::destroy_pipeline(vkb::DispatchTable &dispatch) {
    dispatch.destroyRenderPass(m_render_pass, nullptr);
    dispatch.destroyPipeline(m_pipeline, nullptr);
    dispatch.destroyPipelineLayout(m_pipeline_layout, nullptr);
    m_vert_shader.destroy_shader(dispatch);
    m_frag_shader.destroy_shader(dispatch);
}

VkPipelineDynamicStateCreateInfo Pipeline::get_dynamic_state_create_info() {
    m_dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    info.dynamicStateCount = static_cast<uint32_t>(m_dynamic_states.size());
    info.pDynamicStates = m_dynamic_states.data();

    return info;
}

VkPipelineVertexInputStateCreateInfo Pipeline::get_vertex_input_state_create_info() {
    VkPipelineVertexInputStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.vertexBindingDescriptionCount= 0 ;
    info.vertexAttributeDescriptionCount = 0;
    
    return info;
}

VkPipelineInputAssemblyStateCreateInfo Pipeline::get_input_assembly_state_create_info() {
    VkPipelineInputAssemblyStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    info.primitiveRestartEnable = VK_FALSE;

    return info;
}

VkViewport Pipeline::get_viewport(const vkb::Swapchain &swapchain) {
    VkViewport info{};
    info.x = 0.0f;
    info.y = 0.0f;
    info.width = (float) swapchain.extent.width;
    info.height = (float) swapchain.extent.height;
    info.minDepth = 0.0f;
    info.maxDepth = 1.0f;

    return info;
}

VkRect2D Pipeline::get_scissor(const vkb::Swapchain &swapchain) {
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain.extent;
    
    return scissor;
}

VkPipelineViewportStateCreateInfo Pipeline::get_viewport_state() {
    VkPipelineViewportStateCreateInfo state{};
    state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    state.viewportCount = 1;
    state.scissorCount = 1;
    state.pViewports = nullptr;  // because it's dynamic
    state.pScissors = nullptr;

    return state;
}

VkPipelineRasterizationStateCreateInfo Pipeline::get_rasterizer_state() {
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasClamp = VK_FALSE;

    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo Pipeline::get_multisampling() {
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    return multisampling;
}

VkPipelineColorBlendAttachmentState Pipeline::get_color_blend_attachment() {
    VkPipelineColorBlendAttachmentState attachment{};
    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    attachment.blendEnable = VK_FALSE;

    return attachment;
}

VkPipelineColorBlendStateCreateInfo Pipeline::get_color_blend_state(VkPipelineColorBlendAttachmentState &attachment) {
    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &attachment;

    return color_blending;
}

VkPipelineLayoutCreateInfo Pipeline::get_pipeline_layout_create_info() {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;

    return info;
}

void Pipeline::create_render_pass(vkb::DispatchTable &dispatch, vkb::Swapchain swapchain) {
    // std::cout << "creating attachent desc\n";
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapchain.image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // std::cout << "creating attachent ref\n";
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    // std::cout << "creating subpass desc\n";
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.pPreserveAttachments = nullptr;
    subpass.pInputAttachments = nullptr;

    VkSubpassDependency dependancy{};
    dependancy.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependancy.dstSubpass = 0;
    dependancy.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependancy.srcAccessMask = 0;
    dependancy.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependancy.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    // std::cout << "creating render pass create info\n";
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependancy;
    
    // std::cout << "creating render pass\n";
    if(dispatch.createRenderPass(&render_pass_info, NULL, &m_render_pass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create a render pass!");
}

VkRenderPass Pipeline::get_render_pass() {
    return m_render_pass;
}

VkPipeline Pipeline::get_pipeline() {
    return m_pipeline;
}
}