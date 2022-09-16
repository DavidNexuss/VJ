#pragma once
#include <shambhala.hpp>
namespace shambhala {
namespace util {
simple_vector<uint8_t> createCube();
shambhala::Mesh *meshCreateCube();
shambhala::Model *
modelCreateSkyBox(const simple_vector<shambhala::TextureResource *> &textures);

const char *stacked(GLuint *parameters);
} // namespace util
} // namespace shambhala
