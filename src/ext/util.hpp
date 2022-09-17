#pragma once
#include <shambhala.hpp>
namespace shambhala {
namespace util {
simple_vector<uint8_t> createCube();
shambhala::Shader createScreenVertexShader();
shambhala::Program *createScreenProgram(IResource *fragmentShader);
shambhala::Mesh *meshCreateCube();
shambhala::Mesh *createScreen();
shambhala::Model *
modelCreateSkyBox(const simple_vector<shambhala::TextureResource *> &textures);

const char *stacked(GLuint *parameters);
} // namespace util
} // namespace shambhala
