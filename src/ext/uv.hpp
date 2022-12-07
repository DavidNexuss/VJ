#pragma once
#include <component/video/mesh.hpp>
#include <core/core.hpp>
namespace shambhala {
namespace ui {
std::vector<float> generateUVMap(Mesh **, int count = 1);
std::vector<float> generateUVVertexBufferInterlaces(Mesh *);
} // namespace ui
} // namespace shambhala
