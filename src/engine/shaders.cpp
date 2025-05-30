#include <engine/shaders.h>
#include <vector>
#include <fmt/format.h>

namespace Engine {

void Shader::create_shader(vkb::DispatchTable &dispatch, const std::string& filename, VkShaderStageFlagBits shader_stage) {
    auto shader_code = read_file(filename);

    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shader_code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(shader_code.data());

    if(dispatch.createShaderModule(&create_info, nullptr, &m_shader_module))
        throw std::runtime_error(fmt::format("Failed to create shader from file: {}", filename));

    m_shader_stage_create_info = {};
    m_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_shader_stage_create_info.stage = shader_stage;
    m_shader_stage_create_info.module = m_shader_module;
    m_shader_stage_create_info.pName = "main";
}

VkShaderModule Shader::get_shader() {
    return m_shader_module;
}

VkPipelineShaderStageCreateInfo Shader::get_shader_stage_create_info() {
    return m_shader_stage_create_info;
}

void Shader::destroy_shader(vkb::DispatchTable &dispatch) {
    dispatch.destroyShaderModule(m_shader_module, nullptr);
}

std::vector<char> Shader::read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if(!file.is_open()) 
        throw std::runtime_error(fmt::format("Failed to open file: {}", filename));

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
}

}