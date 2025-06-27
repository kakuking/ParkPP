#pragma once

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

namespace Engine {

class Renderer;

class Texture {
public:
    void init_texture(std::string filename) { m_filename = filename; }
    void create_texture_image(Renderer &renderer);
    void cleanup(vkb::DispatchTable &dispatch_table);

    VkImageView get_image_view() { return m_texture_image_view; }
    VkSampler get_sampler() { return m_texture_sampler; }
private:

    void create_image(Renderer &renderer, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transition_image_layout(Renderer &renderer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
    VkImageView create_image_view(Renderer &renderer, VkImage image, VkFormat format);
    void create_texture_sampler(Renderer &renderer);

    std::string m_filename;
    VkImage m_texture_image;
    VkImageView m_texture_image_view;
    VkSampler m_texture_sampler;
    VkDeviceMemory m_texture_image_memory;

};

}