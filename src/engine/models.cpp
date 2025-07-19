#include <engine/models.h>
#include <fmt/format.h>

namespace Engine {

void Model::create_buffers(Engine::Renderer &renderer) {
    vertex_buffer_size = sizeof(vertices[0]) * vertices.size();
    vertex_buffer_idx = renderer.create_vertex_buffer(vertex_buffer_size);

    index_buffer_size = sizeof(indices[0]) * indices.size();
    index_buffer_idx = renderer.create_index_buffer(index_buffer_size);

    renderer.update_buffer(vertex_buffer_idx, (void*)vertices.data(), vertex_buffer_size);
    renderer.update_buffer(index_buffer_idx, (void*)indices.data(), index_buffer_size);
}

void Model::refresh_buffers(Engine::Renderer &renderer) {
    renderer.update_buffer(vertex_buffer_idx, (void*)vertices.data(), vertex_buffer_size);
    renderer.update_buffer(index_buffer_idx, (void*)indices.data(), index_buffer_size);
}


Model load_obj_model(std::string filename, float base_texture) {
    Model model_info{};

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
        throw std::runtime_error(warn + err);
    
    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    for(const auto &shape: shapes) {
        for(const auto &index: shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.texcoord_index >= 0) {
                vertex.u = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.v = 1.f - attrib.texcoords[2 * index.texcoord_index + 1];
            } else {
                vertex.u = 0.0f;
                vertex.v = 0.0f;
            }

            vertex.color = {1.f, 1.f, 1.f};

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
            
            if(unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(model_info.vertices.size());
                model_info.vertices.push_back(vertex);
            }

            model_info.indices.push_back(unique_vertices[vertex]);
        }
    }

    std::cout << "Loaded model --> ";
    fmt::println("Filename: {}, Num Vertices: {}, num indices: {}", filename, model_info.vertices.size(), model_info.indices.size());
    
    return model_info;
}

Model load_obj_model_with_material(std::string filename, float base_texture, std::string base_dir) {
    Model model_info{};
    model_info.base_texture = base_texture;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (base_dir == "")
        base_dir = filename.substr(0, filename.find_last_of("/\\") + 1);

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), base_dir.c_str()))
        throw std::runtime_error(warn + err);

    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    for(const auto &shape: shapes) {
        const auto &indices = shape.mesh.indices;
        const auto &material_ids = shape.mesh.material_ids;

        for(size_t i = 0; i < indices.size(); ++i) {
            const auto &index = indices[i];

            size_t face_idx = i / 3;
            int mat_id = -1;
            if(face_idx < material_ids.size())
                mat_id = material_ids[face_idx];  // material id for this face

            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.texcoord_index >= 0) {
                vertex.u = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.v = 1.f - attrib.texcoords[2 * index.texcoord_index + 1];
            } else {
                vertex.u = 0.0f;
                vertex.v = 0.0f;
            }

            vertex.color = {1.f, 1.f, 1.f};

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            } else {
                vertex.normal = {0.f, 0.f, 0.f};
            }

            vertex.material_idx = mat_id >= 0 ? base_texture + static_cast<float>(mat_id): base_texture;

            if(unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(model_info.vertices.size());
                model_info.vertices.push_back(vertex);
            }

            model_info.indices.push_back(unique_vertices[vertex]);
        }
    }

    // blender uses a different coordinate system, to follow that we need to apply this
    size_t num_faces = model_info.indices.size() / 3;
    model_info.model_matrix = glm::rotate(glm::mat4(1.f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    std::cout << "Loaded model --> ";
    fmt::println("Filename: {}, Num Vertices: {}, Num Indices: {}, Num Faces: {}", filename, model_info.vertices.size(), model_info.indices.size(), num_faces);

    return model_info;
}
}