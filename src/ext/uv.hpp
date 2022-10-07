#pragma once
#include <core/core.hpp>
#include <shambhala.hpp>
namespace shambhala {
namespace ui {
simple_vector<float> generateUVMap(Mesh **, int count = 1);
simple_vector<float> generateUVVertexBufferInterlaces(Mesh *);
} // namespace ui
} // namespace shambhala
