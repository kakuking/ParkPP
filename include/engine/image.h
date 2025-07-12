#pragma once

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

namespace Engine {

class Renderer;

struct TextureImage {
    std::string m_filename;
    VkImage m_image;
    VkImageView m_image_view;
    VkDeviceMemory m_image_memory;
    VkSampler m_sampler;

    void cleanup(vkb::DispatchTable &dispatch_table);
};

struct TextureImageArray {
    std::vector<std::string> m_filenames;
    VkImage m_image;
    VkImageView m_image_view;
    VkDeviceMemory m_image_memory;
    VkSampler m_sampler;

    uint32_t m_width, m_height;
    uint32_t layer_count = 1;

    void cleanup(vkb::DispatchTable &dispatch_table);
};

struct ColorImage {
    VkImage m_image;
    VkDeviceMemory m_image_memory;
    VkImageView m_image_view;

    void cleanup(vkb::DispatchTable &dispatch_table);
};

struct DepthImage {
    VkImage m_image;
    VkDeviceMemory m_image_memory;
    VkImageView m_image_view;

    void cleanup(vkb::DispatchTable &dispatch_table);
};

class Image {
public:
    static TextureImage create_texture_image(std::string filename);
    static TextureImageArray create_texture_image_array(std::vector<std::string> m_filenames, uint32_t width, uint32_t height, uint32_t layer_count);
    static DepthImage create_depth_image(Renderer &renderer, uint32_t width, uint32_t height, VkFormat depth_format);
    static ColorImage create_color_image(Renderer &renderer, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits num_samples);

    static void initialize_texture_image(Renderer &renderer, TextureImage &texture_image);
    static void initialize_texture_image_array(Renderer &renderer, TextureImageArray &texture_image_array);
private:

    static void create_image(Renderer &renderer, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkSampleCountFlagBits num_samples, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t layer_count=1);
    static void transition_image_layout(Renderer &renderer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t layer_count=1);
    static VkImageView create_image_view(Renderer &renderer, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);
    static VkImageView create_image_array_view(Renderer &renderer, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t layer_count=1);
    static VkSampler create_texture_sampler(Renderer &renderer);

    static bool has_stencil_component(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
};

}