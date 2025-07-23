#include <engine/scene.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <fmt/format.h>

namespace Engine {

bool ends_with(const std::string& value, const std::string& suffix) {
    if (suffix.size() > value.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

ModelInfo Scene::add_model(std::string filename, std::vector<std::string> texture_filename, bool opaque, bool updating) {
    if(ends_with(filename, ".obj"))
        return add_obj_model(filename, texture_filename, opaque, updating);
    else if(ends_with(filename, ".glb") || ends_with(filename, ".gltf"))
        return add_gltf_model(filename, texture_filename, opaque, updating);
    else
        throw std::runtime_error("Unsupported model format!");
}


/*
ModelInfo Scene::add_obj_model(std::string filename, std::vector<std::string> texture_filename, bool opaque, bool updating) {
    if(updating)
        std::cout << "Updating\n";
    
    float base_texture = (float)m_textures.size();

    m_textures.insert(m_textures.end(), texture_filename.begin(), texture_filename.end());

    Model model{};

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
        throw std::runtime_error(warn + err);

    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    for (const auto &shape : shapes) {
        for (const auto &index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.texcoord_index >= 0) {
                vertex.u = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.v += 1.f - attrib.texcoords[2 * index.texcoord_index + 1];
            } else {
                vertex.u = 0.0f;
                vertex.v += 0.0f;
            }

            vertex.color = {1.f, 1.f, (float)m_model_transform_matrices.size()};

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            } else {
                vertex.normal = {0.f, 0.f, 0.f};
            }

            vertex.material_idx = base_texture;

            if (unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(model.vertices.size());
                model.vertices.push_back(vertex);
            }

            model.indices.push_back(unique_vertices[vertex]);
        }
    }

    model.model_matrix = glm::rotate(glm::mat4(1.f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    ModelInfo model_info{};
    if (opaque) {
        m_opaque_models.push_back(model);

        model_info.model_idx = m_opaque_models.size() - 1;
        model_info.model_sub_idx = 0;
        model_info.model_transform_idx = m_model_transform_matrices.size() - 1;
    }
    else {
        m_transparent_models.push_back(model);

        model_info.model_idx = m_transparent_models.size() - 1;
        model_info.model_sub_idx = 0;
        model_info.model_transform_idx = m_model_transform_matrices.size() - 1;
    }

    m_model_transform_matrices.push_back(model.model_matrix);
    model_info.model_sub_idx = 0;
    model_info.model_transform_idx = m_model_transform_matrices.size() - 1;

    fmt::println("Loaded model --> Filename: {}, Vertices: {}, Indices: {}",
                 filename, model.vertices.size(), model.indices.size());

    return model_info;
}
*/

ModelInfo Scene::add_obj_model(std::string filename, std::vector<std::string> texture_filename, bool opaque, bool updating) {
    if(updating)
        std::cout << "Updating\n";
    
    float base_texture = (float)m_textures.size();

    m_textures.insert(m_textures.end(), texture_filename.begin(), texture_filename.end());

    Model model{};
    model.base_texture = base_texture;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string base_dir = filename.substr(0, filename.find_last_of("/\\") + 1);

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), base_dir.c_str()))
        throw std::runtime_error(warn + err);

    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    for (const auto &shape : shapes) {
        const auto &indices = shape.mesh.indices;
        const auto &material_ids = shape.mesh.material_ids;

        for (size_t i = 0; i < indices.size(); ++i) {
            const auto &index = indices[i];

            size_t face_idx = i / 3;
            int mat_id = -1;
            if (face_idx < material_ids.size())
                mat_id = material_ids[face_idx];

            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.texcoord_index >= 0) {
                vertex.u = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.v += 1.f - attrib.texcoords[2 * index.texcoord_index + 1];
            } else {
                vertex.u = 0.0f;
                vertex.v += 0.0f;
            }

            vertex.color = {1.f, 1.f, (float)m_model_transform_matrices.size()};

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            } else {
                vertex.normal = {0.f, 0.f, 0.f};
            }

            vertex.material_idx = mat_id >= 0 ? base_texture + static_cast<float>(mat_id) : base_texture;

            if (unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(model.vertices.size());
                model.vertices.push_back(vertex);
            }

            model.indices.push_back(unique_vertices[vertex]);
        }
    }

    model.model_matrix = glm::rotate(glm::mat4(1.f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    ModelInfo model_info{};
    if (opaque) {
        m_opaque_models.push_back(model);

        model_info.model_idx = m_opaque_models.size() - 1;
        model_info.model_sub_idx = 0;
        model_info.model_transform_idx = m_model_transform_matrices.size() - 1;
    }
    else {
        m_transparent_models.push_back(model);

        model_info.model_idx = m_transparent_models.size() - 1;
        model_info.model_sub_idx = 0;
        model_info.model_transform_idx = m_model_transform_matrices.size() - 1;
    }

    m_model_transform_matrices.push_back(model.model_matrix);
    model_info.model_sub_idx = 0;
    model_info.model_transform_idx = m_model_transform_matrices.size() - 1;

    size_t num_faces = model.indices.size() / 3;
    fmt::println("Loaded model --> Filename: {}, Num Vertices: {}, Num Indices: {}, Num Faces: {}",
                 filename, model.vertices.size(), model.indices.size(), num_faces);

    return model_info;
}

ModelInfo Scene::add_gltf_model(std::string filename, std::vector<std::string> texture_filename, bool opaque, bool updating) {
    if (updating)
        std::cout << "Updating\n";

    float base_texture = (float)m_textures.size();
    m_textures.insert(m_textures.end(), texture_filename.begin(), texture_filename.end());

    Model model{};
    model.base_texture = base_texture;

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ret;
    if (ends_with(filename, ".glb"))
        ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename);
    else 
        ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filename);

    if (!ret)
        throw std::runtime_error("Failed to load GLTF: " + warn + err);

    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    for (const auto &gltfMesh : gltfModel.meshes) {
        for (const auto &primitive : gltfMesh.primitives) {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES) continue;

            const auto &posAccessor = gltfModel.accessors[primitive.attributes.at("POSITION")];
            const auto &posBufferView = gltfModel.bufferViews[posAccessor.bufferView];
            const auto &posBuffer = gltfModel.buffers[posBufferView.buffer];

            const float* positions = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);
            
            const float* normals = nullptr;
            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                const auto &normalAccessor = gltfModel.accessors[primitive.attributes.at("NORMAL")];
                const auto &normalView = gltfModel.bufferViews[normalAccessor.bufferView];
                normals = reinterpret_cast<const float*>(&gltfModel.buffers[normalView.buffer].data[normalView.byteOffset + normalAccessor.byteOffset]);
            }

            const float* texcoords = nullptr;
            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                const auto &uvAccessor = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
                const auto &uvView = gltfModel.bufferViews[uvAccessor.bufferView];
                texcoords = reinterpret_cast<const float*>(&gltfModel.buffers[uvView.buffer].data[uvView.byteOffset + uvAccessor.byteOffset]);
            }

            const auto &indexAccessor = gltfModel.accessors[primitive.indices];
            const auto &indexView = gltfModel.bufferViews[indexAccessor.bufferView];
            const auto &indexBuffer = gltfModel.buffers[indexView.buffer];

            const void* indexData = &indexBuffer.data[indexView.byteOffset + indexAccessor.byteOffset];
            const size_t indexCount = indexAccessor.count;

            int mat_id = primitive.material;

            for (size_t i = 0; i < indexCount; ++i) {
                uint32_t index = 0;
                switch (indexAccessor.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                        index = ((const uint8_t*)indexData)[i]; break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                        index = ((const uint16_t*)indexData)[i]; break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                        index = ((const uint32_t*)indexData)[i]; break;
                }

                Vertex vertex{};

                vertex.pos = {
                    positions[3 * index + 0],
                    positions[3 * index + 1],
                    positions[3 * index + 2]
                };

                vertex.normal = normals ? glm::vec3(
                    normals[3 * index + 0],
                    normals[3 * index + 1],
                    normals[3 * index + 2]
                ) : glm::vec3(0.f);

                if (texcoords) {
                    vertex.u = texcoords[2 * index + 0];
                    vertex.v = texcoords[2 * index + 1]; // Flip V
                } else {
                    vertex.u = 0.0f;
                    vertex.v = 0.0f;
                }

                vertex.color = {1.f, 1.f, (float)m_model_transform_matrices.size()};
                vertex.material_idx = mat_id >= 0 ? base_texture + static_cast<float>(mat_id) : base_texture;

                if (unique_vertices.count(vertex) == 0) {
                    unique_vertices[vertex] = static_cast<uint32_t>(model.vertices.size());
                    model.vertices.push_back(vertex);
                }

                model.indices.push_back(unique_vertices[vertex]);
            }
        }
    }

    model.model_matrix = glm::rotate(glm::mat4(1.f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    ModelInfo model_info{};
    if (opaque) {
        m_opaque_models.push_back(model);
        model_info.model_idx = m_opaque_models.size() - 1;
    } else {
        m_transparent_models.push_back(model);
        model_info.model_idx = m_transparent_models.size() - 1;
    }

    m_model_transform_matrices.push_back(model.model_matrix);
    model_info.model_transform_idx = m_model_transform_matrices.size() - 1;
    model_info.model_sub_idx = 0;

    size_t num_faces = model.indices.size() / 3;
    fmt::println("Loaded GLTF --> Filename: {}, Vertices: {}, Indices: {}, Faces: {}",
                 filename, model.vertices.size(), model.indices.size(), num_faces);

    return model_info;
}

void Scene::create_buffers(Renderer &renderer) {
    for (Model &model: m_opaque_models)
        model.create_buffers(renderer);

    for (Model &model: m_transparent_models)
        model.create_buffers(renderer);
    
    size_t transform_count = m_model_transform_matrices.size();
    uint32_t buffer_size = sizeof(glm::mat4) * static_cast<uint32_t>(transform_count);

    size_t ub_idx = renderer.create_uniform_group(1, buffer_size, VK_SHADER_STAGE_VERTEX_BIT, true);
    renderer.update_uniform_group(ub_idx, m_model_transform_matrices.data());
    
    // renderer.add_texture("textures/viking_room.jpg", 1);
    uint32_t tex_count = static_cast<uint32_t>(num_textures());
    uint32_t layer_count = tex_count < 4 ? 4: tex_count;
    renderer.add_texture_array(m_textures, 1024, 1024, layer_count, 2);
}

void Scene::render_opaque_models(Renderer &renderer, VkCommandBuffer &command_buffer) {
    for (int mod = 0; mod < m_opaque_models.size(); mod++) {
        // std::cout << "Rendering model\n";
        const Engine::Model &model = m_opaque_models[mod];

        // Retreive vertex and index buffers =============================================================
        VkBuffer vertex_buffers[] = {renderer.get_buffer(model.vertex_buffer_idx)};
        VkDeviceSize offsets[] = {0};
        VkBuffer index_buffer = renderer.get_buffer(model.index_buffer_idx);
        
        // Bind vertex and index buffers =================================================================
        renderer.m_dispatch.cmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        renderer.m_dispatch.cmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

        // Set push constants ============================================================================
        renderer.m_dispatch.cmdPushConstants(command_buffer, renderer.get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(m_push_constants), &m_push_constants);
        
        // draw call =====================================================================================
        renderer.m_dispatch.cmdDrawIndexed(command_buffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
    }
}

void Scene::render_transparent_models(Renderer &renderer, VkCommandBuffer &command_buffer) {
    for (int mod = 0; mod < m_transparent_models.size(); mod++) {
        const Engine::Model &model = m_transparent_models[mod];

        // Retreive vertex and index buffers =============================================================
        VkBuffer vertex_buffers[] = {renderer.get_buffer(model.vertex_buffer_idx)};
        VkDeviceSize offsets[] = {0};
        VkBuffer index_buffer = renderer.get_buffer(model.index_buffer_idx);
        
        // Bind vertex and index buffers =================================================================
        renderer.m_dispatch.cmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        renderer.m_dispatch.cmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

        // Set push constants ============================================================================
        renderer.m_dispatch.cmdPushConstants(command_buffer, renderer.get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(m_push_constants), &m_push_constants);
        
        // draw call =====================================================================================
        renderer.m_dispatch.cmdDrawIndexed(command_buffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
    }
}

int Scene::add_light(Renderer &renderer, glm::vec3 light_pos, glm::mat4 light_matrix) {
    int light_idx = renderer.add_light(light_matrix, 0);

    m_lights.push_back(light_idx);

    m_push_constants.light = light_matrix;
    m_push_constants.light_pos = glm::vec4(light_pos, 0.f);

    return light_idx;
}

int Scene::add_orthographic_light(Renderer &renderer, glm::vec3 position, glm::vec3 look_at, glm::vec3 up, float near_plane, float far_plane, float ortho_half_size) {
    glm::vec3 light_pos = position; // light above and at an angle
    glm::vec3 light_target = look_at;
    glm::vec3 light_up = up;

    glm::mat4 light_view = glm::lookAt(light_pos, light_target, light_up);

    glm::mat4 light_proj = glm::ortho(
        -ortho_half_size, ortho_half_size,
        -ortho_half_size, ortho_half_size,
        near_plane, far_plane
    );

    light_proj[1][1] *= -1;
    glm::mat4 light_pv = light_proj * light_view;

    return add_light(renderer, position, light_pv);
}

void Scene::set_camera(glm::mat4 proj, glm::mat4 view) {
    m_push_constants.proj = proj;
    m_push_constants.view = view;
}

void Scene::set_perspective_camera(glm::vec3 eye, glm::vec3 look_at, glm::vec3 up, float aspect_ratio, float near_plane, float far_plane, float fov_degrees) {
    m_push_constants.view = glm::lookAt(eye, look_at, up);
    m_push_constants.proj = glm::perspective(glm::radians(fov_degrees), aspect_ratio, near_plane, far_plane);
    m_push_constants.proj[1][1] *= -1;
}

void Scene::update_opaque_model_transform(ModelInfo &mi, glm::mat4 transform, bool replace) {
    if (replace)
        m_opaque_models[mi.model_idx].model_matrix = transform;
    else
        m_opaque_models[mi.model_idx].model_matrix = transform * m_opaque_models[mi.model_idx].model_matrix;
        
    m_model_transform_matrices[mi.model_transform_idx] = m_opaque_models[mi.model_idx].model_matrix;
}

void Scene::update_transparent_model_transform(ModelInfo &mi, glm::mat4 transform, bool replace) {
    if (replace)
        m_transparent_models[mi.model_idx].model_matrix = transform;
    else
        m_transparent_models[mi.model_idx].model_matrix = transform * m_transparent_models[mi.model_idx].model_matrix;

    m_model_transform_matrices[mi.model_transform_idx] = m_transparent_models[mi.model_idx].model_matrix;
}

void Scene::update(float delta_time) {
    total_time += delta_time;

    float camera_d = 10.f;

    m_push_constants.view = glm::lookAt(glm::vec3(camera_d * cosf(total_time), camera_d * sinf(total_time),  camera_d), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
}
}