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
}