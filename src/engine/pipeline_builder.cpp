#include <engine/pipeline_builder.h>
#include <engine/renderer.h>

namespace Engine {

PipelineData PipelineBuilder::build(Renderer &renderer) {
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
    // std::cout << "Creating viewport state\n";
    VkPipelineViewportStateCreateInfo viewport_state = get_viewport_state(VkExtent2D{1024, 1024}); // only for shadow maps anywats
    // std::cout << "Creating rast\n";
    VkPipelineRasterizationStateCreateInfo rasterizer = get_rasterizer_state();
    // std::cout << "Creating ms\n";
    VkPipelineMultisampleStateCreateInfo multisampling = get_multisampling(renderer.get_msaa_sample_count());
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
    if(renderer.m_dispatch.createGraphicsPipelines(VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error("Could not create graphics pipeline!");

    PipelineData ret{};
    ret.frag_shader = m_frag_shader;
    ret.vert_shader = m_vert_shader;
    ret.pipeline = pipeline;
    ret.pipeline_layout = m_pipeline_layout;
    ret.render_pass = m_render_pass;
    ret.m_unique_render_pass = m_unique_render_pass;
    
    return ret;
}

void PipelineBuilder::create_pipeline_layout(Renderer &device, std::vector<VkDescriptorSetLayout> descriptor_set_layouts) {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
    info.pSetLayouts = descriptor_set_layouts.data();
    info.pushConstantRangeCount = static_cast<uint32_t>(m_push_constant_ranges.size());
    info.pPushConstantRanges = m_push_constant_ranges.data();

    if(device.m_dispatch.createPipelineLayout(&info, nullptr, &m_pipeline_layout) != VK_SUCCESS)
        throw std::runtime_error("Could not create pipeline layout");    
}

void PipelineBuilder::set_shaders(Renderer &device, const std::string &vert_shader_filename, const std::string &frag_shader_filename) {
    // std::cout << "Creating vertex shader\n";
    m_vert_shader.create_shader(device.m_dispatch, vert_shader_filename, VK_SHADER_STAGE_VERTEX_BIT);
    // std::cout << "Creating frag shader\n";
    m_frag_shader.create_shader(device.m_dispatch, frag_shader_filename, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void PipelineBuilder::add_push_constants(uint32_t pc_size, uint32_t offset, VkShaderStageFlags shader_stage) {
    VkPushConstantRange push_constant;
	push_constant.offset = offset;
	push_constant.size = pc_size;
	push_constant.stageFlags = shader_stage;

    m_push_constant_ranges.push_back(push_constant);
}


void PipelineBuilder::create_render_pass(Renderer &renderer, vkb::Swapchain swapchain, VkRenderPass old_render_pass) {
    if (old_render_pass != VK_NULL_HANDLE) {
        m_render_pass = old_render_pass;
        m_unique_render_pass = false;
        return;
    }

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain.image_format;
    colorAttachment.samples = renderer.get_msaa_sample_count();
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = renderer.find_depth_format();
    depth_attachment.samples = renderer.get_msaa_sample_count();
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription color_attachment_resolve{};
    color_attachment_resolve.format = renderer.get_swapchain().image_format;
    color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_resolve_ref{};
    color_attachment_resolve_ref.attachment = 2;
    color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;
    subpass.pResolveAttachments = &color_attachment_resolve_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments = {colorAttachment, depth_attachment, color_attachment_resolve};

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

void PipelineBuilder::create_shadow_render_pass(Renderer &renderer) {
    VkFormat depth_format = renderer.find_depth_format();

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = depth_format;
    depth_attachment.samples = m_enable_msaa ? renderer.get_msaa_sample_count(): VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at beginning of the render pass
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// We will read from depth, so it's important to store the depth attachment results
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 0;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;            // No color attachments here
    subpass.pColorAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    std::vector<VkSubpassDependency> dependencies(2);

    // External → Shadow Pass: allow write access to depth
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Shadow Pass → External: allow shader sampling after depth write
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &depth_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    render_pass_info.pDependencies = dependencies.data();

    if (renderer.m_dispatch.createRenderPass(&render_pass_info, nullptr, &m_render_pass) != VK_SUCCESS) 
        throw std::runtime_error("failed to create shadow render pass!");
} 


VkPipelineDepthStencilStateCreateInfo PipelineBuilder::get_depth_stencil_create_info() {
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = m_enable_depth_test ? VK_TRUE : VK_FALSE;
    depth_stencil.depthWriteEnable = m_enable_depth_write ? VK_TRUE : VK_FALSE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    return depth_stencil;
}

VkPipelineDynamicStateCreateInfo PipelineBuilder::get_dynamic_state_create_info() {
    if (m_enable_dynamic_state)
        m_dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
    else
        m_dynamic_states = {};

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

VkViewport PipelineBuilder::get_viewport(float width, float height) {
    VkViewport info{};
    info.x = 0.0f;
    info.y = 0.0f;
    info.width = width;
    info.height = height;
    info.minDepth = 0.0f;
    info.maxDepth = 1.0f;

    return info;
}

VkRect2D PipelineBuilder::get_scissor(uint32_t width, uint32_t height) {
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = VkExtent2D{width, height};
    
    return scissor;
}

VkPipelineViewportStateCreateInfo PipelineBuilder::get_viewport_state(VkExtent2D extent) {

    m_viewport = get_viewport((float)extent.width, (float)extent.height);
    m_scissor = get_scissor(extent.width, extent.height);

    VkPipelineViewportStateCreateInfo state{};
    state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    state.viewportCount = 1;
    state.scissorCount = 1;
    state.pViewports = m_enable_dynamic_state ? nullptr: &m_viewport;  // because it's dynamic
    state.pScissors = m_enable_dynamic_state ? nullptr: &m_scissor;

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

    rasterizer.depthBiasEnable = VK_TRUE;
    // rasterizer.depthBiasConstantFactor = 1.25f;
    rasterizer.depthBiasConstantFactor = 2.5f;
	rasterizer.depthBiasSlopeFactor = 1.75f;
    rasterizer.depthBiasClamp = 0.0f;

    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo PipelineBuilder::get_multisampling(VkSampleCountFlagBits num_samples) {
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = m_enable_msaa ? num_samples: VK_SAMPLE_COUNT_1_BIT;

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
    color_blending.attachmentCount = m_use_color_attachment ? 1: 0;
    color_blending.pAttachments = m_use_color_attachment ? &attachment: nullptr;

    return color_blending;
}

VkPipeline build(Renderer &device);

}