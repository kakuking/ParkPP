#pragma once
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include <fstream>
#include <vector>

namespace Engine {

class Shader {
public:
    void create_shader(vkb::DispatchTable &dispatch, const std::string& filename, VkShaderStageFlagBits shader_stage);
    VkShaderModule get_shader();
    VkPipelineShaderStageCreateInfo get_shader_stage_create_info();
    void destroy_shader(vkb::DispatchTable &dispatch);
    
private:
    static std::vector<char> read_file(const std::string& filename);

    VkShaderModule m_shader_module;
    VkPipelineShaderStageCreateInfo m_shader_stage_create_info;
};

}