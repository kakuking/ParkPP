#include <engine/pipeline_builder.h>
#include <engine/renderer.h>

namespace Engine {

PipelineData PipelineBuilder::build(Renderer &device) {
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

    VkPipelineDepthStencilStateCreateInfo depth_stencil = get_depth_stencil_create_info();

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
    create_info.pDepthStencilState = &depth_stencil;
    create_info.pColorBlendState = &color_blending;
    create_info.pDynamicState = &dynamic_states;
    create_info.layout = m_pipeline_layout;
    create_info.renderPass = m_render_pass;
    create_info.subpass = 0;
    create_info.basePipelineHandle = VK_NULL_HANDLE;
    create_info.basePipelineIndex = -1;

    VkPipeline pipeline;
    // std::cout << "Creating pipeline\n";
    if(device.m_dispatch.createGraphicsPipelines(VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Could not create graphics pipeline!");

    PipelineData ret{};
    ret.frag_shader = m_frag_shader;
    ret.vert_shader = m_vert_shader;
    ret.pipeline = pipeline;
    ret.pipeline_layout = m_pipeline_layout;
    ret.render_pass = m_render_pass;
    
    return ret;
}

void PipelineBuilder::create_pipeline_layout(Renderer &device, VkDescriptorSetLayout *descriptor_set_layouts) {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = 1;
    info.pSetLayouts = descriptor_set_layouts;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;

    if(device.m_dispatch
        .createPipelineLayout(&info, nullptr, &m_pipeline_layout) != VK_SUCCESS)
        throw std::runtime_error("Could not create pipeline layout");    
}

void PipelineBuilder::set_shaders(Renderer &device, const std::string &vert_shader_filename, const std::string &frag_shader_filename) {
    // std::cout << "Creating vertex shader\n";
    m_vert_shader.create_shader(device.m_dispatch, vert_shader_filename, VK_SHADER_STAGE_VERTEX_BIT);
    // std::cout << "Creating frag shader\n";
    m_frag_shader.create_shader(device.m_dispatch, frag_shader_filename, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void PipelineBuilder::create_render_pass(Renderer &renderer, vkb::Swapchain swapchain) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain.image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = renderer.find_depth_format();
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments = {colorAttachment, depth_attachment};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (renderer.m_dispatch.createRenderPass(&renderPassInfo, nullptr, &m_render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

VkPipelineDepthStencilStateCreateInfo PipelineBuilder::get_depth_stencil_create_info() {
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    return depth_stencil;
}

VkPipelineDynamicStateCreateInfo PipelineBuilder::get_dynamic_state_create_info() {
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

VkPipelineVertexInputStateCreateInfo PipelineBuilder::get_vertex_input_state_create_info() {

    VkPipelineVertexInputStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.vertexBindingDescriptionCount = 1;
    info.pVertexBindingDescriptions = &m_vertex_binding_desc;

    info.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_vertex_input_attr_desc.size());
    info.pVertexAttributeDescriptions = m_vertex_input_attr_desc.data();
    
    return info;
}

VkPipelineInputAssemblyStateCreateInfo PipelineBuilder::get_input_assembly_state_create_info() {
    VkPipelineInputAssemblyStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.topology = m_input_topology;
    info.primitiveRestartEnable = VK_FALSE;

    return info;
}

VkViewport PipelineBuilder::get_viewport(const vkb::Swapchain &swapchain) {
    VkViewport info{};
    info.x = 0.0f;
    info.y = 0.0f;
    info.width = (float) swapchain.extent.width;
    info.height = (float) swapchain.extent.height;
    info.minDepth = 0.0f;
    info.maxDepth = 1.0f;

    return info;
}

VkRect2D PipelineBuilder::get_scissor(const vkb::Swapchain &swapchain) {
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain.extent;
    
    return scissor;
}

VkPipelineViewportStateCreateInfo PipelineBuilder::get_viewport_state() {
    VkPipelineViewportStateCreateInfo state{};
    state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    state.viewportCount = 1;
    state.scissorCount = 1;
    state.pViewports = nullptr;  // because it's dynamic
    state.pScissors = nullptr;

    return state;
}

VkPipelineRasterizationStateCreateInfo PipelineBuilder::get_rasterizer_state() {
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = m_polygon_mode;
    rasterizer.lineWidth = 1.f;
    rasterizer.cullMode = m_cull_mode;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasClamp = VK_FALSE;

    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo PipelineBuilder::get_multisampling() {
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    return multisampling;
}

VkPipelineColorBlendAttachmentState PipelineBuilder::get_color_blend_attachment() {
    VkPipelineColorBlendAttachmentState attachment{};
    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    if(!m_enable_blending) {
        attachment.blendEnable = VK_FALSE;
    } else {
        attachment.blendEnable = VK_TRUE;
        attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        attachment.colorBlendOp = VK_BLEND_OP_ADD;
    }

    return attachment;
}

VkPipelineColorBlendStateCreateInfo PipelineBuilder::get_color_blend_state(VkPipelineColorBlendAttachmentState &attachment) {
    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &attachment;

    return color_blending;
}

VkPipeline build(Renderer &device);

}