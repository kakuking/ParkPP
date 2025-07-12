#include <game/models.h>
#include <fmt/format.h>

namespace Game {

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


Model load_obj_model(std::string filename) {
    Model model_info{};

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::cout << "Loading model\n";
    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
    throw std::runtime_error(warn + err);
    
    std::cout << "Loaded model\n";
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

            
            if(unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(model_info.vertices.size());
                model_info.vertices.push_back(vertex);
            }

            model_info.indices.push_back(unique_vertices[vertex]);
        }
    }

    std::cout << "Returning model\n";
    fmt::println("Num Vertices: {}, num indices: {}\n\n", model_info.vertices.size(), model_info.indices.size());
    
    return model_info;
}
}