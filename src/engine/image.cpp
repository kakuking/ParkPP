// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <engine/image.h>
#include <engine/renderer.h>
#include <iostream>

namespace Engine {

void TextureImage::cleanup(vkb::DispatchTable &dispatch_table) {
    dispatch_table.destroySampler(m_sampler, nullptr);
    dispatch_table.destroyImageView(m_image_view, nullptr);
    dispatch_table.destroyImage(m_image, nullptr);
    dispatch_table.freeMemory(m_image_memory, nullptr);
}

void ShadowMapImage::cleanup(vkb::DispatchTable &dispatch_table) {
    dispatch_table.destroySampler(m_sampler, nullptr);
    dispatch_table.destroyImageView(m_image_view, nullptr);
    dispatch_table.destroyImage(m_image, nullptr);
    dispatch_table.freeMemory(m_image_memory, nullptr);
}

void TextureImageArray::cleanup(vkb::DispatchTable &dispatch_table) {
    dispatch_table.destroySampler(m_sampler, nullptr);
    dispatch_table.destroyImageView(m_image_view, nullptr);
    dispatch_table.destroyImage(m_image, nullptr);
    dispatch_table.freeMemory(m_image_memory, nullptr);
}

void DepthImage::cleanup(vkb::DispatchTable &dispatch_table) {
    dispatch_table.destroyImageView(m_image_view, nullptr);
    dispatch_table.destroyImage(m_image, nullptr);
    dispatch_table.freeMemory(m_image_memory, nullptr);
}

void ColorImage::cleanup(vkb::DispatchTable &dispatch_table) {
    dispatch_table.destroyImageView(m_image_view, nullptr);
    dispatch_table.destroyImage(m_image, nullptr);
    dispatch_table.freeMemory(m_image_memory, nullptr);
}

TextureImage Image::create_texture_image(std::string filename) {
    TextureImage ret{}; 
    ret.m_filename = filename; 
    return ret; 
}

TextureImageArray Image::create_texture_image_array(std::vector<std::string> filenames, uint32_t width, uint32_t height, uint32_t layer_count) {
    TextureImageArray ret{};
    ret.m_filenames = filenames;
    ret.m_width = width;
    ret.m_height = height;
    ret.layer_count = layer_count;

    return ret;
}


DepthImage Image::create_depth_image(Renderer &renderer, uint32_t width, uint32_t height, VkFormat depth_format) {
    VkImage depth_image;
    VkDeviceMemory depth_image_memory;
    VkImageView depth_image_view;

    create_image(renderer, width, height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, renderer.get_msaa_sample_count(),  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);

    depth_image_view = create_image_view(renderer, depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    transition_image_layout(renderer, depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    DepthImage ret{};
    ret.m_image = depth_image;
    ret.m_image_memory = depth_image_memory;
    ret.m_image_view = depth_image_view;

    return ret;
}

ShadowMapImage Image::create_shadow_map_image(Renderer &renderer, uint32_t width, uint32_t height, VkFormat depth_format, uint32_t layer_count) {
    VkImage depth_image;
    VkDeviceMemory depth_image_memory;
    VkImageView depth_image_view;

    create_image(renderer, width, height, depth_format,
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_SAMPLE_COUNT_1_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        depth_image, depth_image_memory, layer_count, 0);
    
    // depth_image_view = create_image_array_view(renderer, depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, layer_count);
    depth_image_view = create_image_array_view(renderer, depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, layer_count);
    
    transition_image_layout(renderer, depth_image, depth_format,
        VK_IMAGE_LAYOUT_UNDEFINED,
        // VK_IMAGE_LAYOUT_GENERAL, layer_count);
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, layer_count);

    ShadowMapImage ret{};
    ret.m_image = depth_image;
    ret.m_image_memory = depth_image_memory;
    ret.m_image_view = depth_image_view;
    ret.m_sampler = create_shadow_map_sampler(renderer);

    return ret;
}

ColorImage Image::create_color_image(Renderer &renderer, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits num_samples) {
    VkImage image;
    VkDeviceMemory image_memory;
    VkImageView image_view;

    create_image(renderer, width, height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, num_samples, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, image_memory);

    image_view = create_image_view(renderer, image, format, VK_IMAGE_ASPECT_COLOR_BIT);

    ColorImage ret{};
    ret.m_image = image;
    ret.m_image_memory = image_memory;
    ret.m_image_view = image_view;

    return ret;
}

void Image::initialize_texture_image(Renderer &renderer, TextureImage &tex) {
    int tex_width, tex_height, tex_channels;

    stbi_uc* pixels = stbi_load(tex.m_filename.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

    VkDeviceSize image_size = tex_width * tex_height * 4;

    if(!pixels)
        throw std::runtime_error("Failed to load Image image!");

    size_t staging_buffer_idx = renderer.create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    renderer.update_buffer(staging_buffer_idx, pixels, static_cast<size_t>(image_size));

    stbi_image_free(pixels);

    create_image(
        renderer, tex_width, tex_height, 
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, tex.m_image, tex.m_image_memory
    );

    transition_image_layout(renderer, tex.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        renderer.copy_buffer_to_image(staging_buffer_idx, tex.m_image, static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));
    transition_image_layout(renderer, tex.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Create image view
    tex.m_image_view = create_image_view(renderer, tex.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    // Create image sampler
    tex.m_sampler = create_texture_sampler(renderer);
}

void Image::initialize_texture_image_array(Renderer &renderer, TextureImageArray &tex) {
    if (tex.m_filenames.empty() || tex.layer_count == 0)
        throw std::runtime_error("TextureImageArray has no filenames or zero layers");

    if(tex.m_filenames.size() > tex.layer_count)
        throw std::runtime_error("TextureImageArray has too many filenames or too few layers");
    
    uint32_t width = tex.m_width;
    uint32_t height = tex.m_height;

    create_image(
        renderer, width, height, 
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, tex.m_image, tex.m_image_memory,
        tex.layer_count
    );

    transition_image_layout(renderer, tex.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, tex.layer_count);

    uint32_t cur_layer = 0;
    for(std::string &filename: tex.m_filenames) {
        int tex_width, tex_height, tex_channels;

        stbi_uc* pixels = stbi_load(filename.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

        if (static_cast<uint32_t>(tex_width) != tex.m_width && static_cast<uint32_t>(tex_height) != tex.m_height)
            throw std::runtime_error("All images in an image array must be the same size!");

        VkDeviceSize image_size = tex_width * tex_height * 4;

        if(!pixels)
            throw std::runtime_error("Failed to load Image image!");

        size_t staging_buffer_idx = renderer.create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        renderer.update_buffer(staging_buffer_idx, pixels, static_cast<size_t>(image_size));

        stbi_image_free(pixels);

        renderer.copy_buffer_to_image(staging_buffer_idx, tex.m_image, static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height), cur_layer);

        cur_layer++;
    }

    transition_image_layout(renderer, tex.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, tex.layer_count);

    // Create image view
    tex.m_image_view = create_image_array_view(renderer, tex.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, tex.layer_count);

    // Create image sampler
    tex.m_sampler = create_texture_sampler(renderer);
}


VkImageView Image::create_image_view(Renderer &renderer, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspect_flags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (renderer.m_dispatch.createImageView(&viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Image image view!");
    }

    return imageView;
}

VkImageView Image::create_image_array_view(Renderer &renderer, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t layer_count) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspect_flags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layer_count;

    VkImageView imageView;
    if (renderer.m_dispatch.createImageView(&viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Image image view!");
    }

    return imageView;
}

void Image::create_image(Renderer &renderer, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkSampleCountFlagBits num_samples, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t layer_count, VkImageCreateFlags flags, VkImageLayout layout) {
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(width);
    image_info.extent.height = static_cast<uint32_t>(height);
    // no mipmapping rn
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = layer_count;

    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = layout;

    image_info.usage = usage;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = num_samples;
    image_info.flags = flags;

    if (renderer.m_dispatch.createImage(&image_info, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image!");
    
    VkMemoryRequirements mem_req;
    renderer.m_dispatch.getImageMemoryRequirements(image, &mem_req);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = renderer.find_memory_type(mem_req.memoryTypeBits, properties);

    if(renderer.m_dispatch.allocateMemory(&alloc_info, nullptr, &imageMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate image memory!");

    renderer.m_dispatch.bindImageMemory(image, imageMemory, 0);
}

void Image::transition_image_layout(Renderer &renderer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t layer_count, VkCommandBuffer command_buffer) {
    bool use_single_time = false;
    if (command_buffer == VK_NULL_HANDLE) {
        use_single_time = true;
        command_buffer = renderer.begin_single_time_command();
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;

    if (format == VK_FORMAT_D16_UNORM ||
        format == VK_FORMAT_D32_SFLOAT ||
        format == VK_FORMAT_D24_UNORM_S8_UINT ||
        format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
        // It's a depth or depth-stencil image
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        
        if (has_stencil_component(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        // It's a color image
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layer_count;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags source_stage, destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; // or EARLY_FRAGMENT_TESTS
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;  // reading as shader input
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;  // still reading as shader input, just different layout

        source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // or whatever stage will use GENERAL layout
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    renderer.m_dispatch.cmdPipelineBarrier(
        command_buffer, 
        source_stage, destination_stage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    
    if (use_single_time)
        renderer.end_single_time_command(command_buffer);
}

VkSampler Image::create_texture_sampler(Renderer &renderer) {
    VkSampler ret;

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = renderer.get_physical_device_properties().limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    if(renderer.m_dispatch.createSampler(&sampler_info, nullptr, &ret) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Image sampler!");

    return ret;
}

VkSampler Image::create_shadow_map_sampler(Renderer &renderer) {
    VkSampler ret;

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    // sampler_info.magFilter = VK_FILTER_NEAREST;
    // sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_TRUE;
    sampler_info.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    if (renderer.m_dispatch.createSampler(&sampler_info, nullptr, &ret) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shadow map sampler!");

    return ret;
}

}