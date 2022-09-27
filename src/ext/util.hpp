#pragma once
#include <shambhala.hpp>
#include <ext.hpp>
#include <standard.hpp>
namespace shambhala {
namespace util {
simple_vector<uint8_t> createCube();
shambhala::Shader createScreenVertexShader();
shambhala::Shader createRegularVertexShader();
shambhala::Shader createEmptyFragmentShader();
shambhala::Program *createScreenProgram(IResource *fragmentShader);
shambhala::Program *createDepthOnlyProgram();
shambhala::Program *createRegularShaderProgram(IResource *fragmentShader);
shambhala::Mesh *meshCreateCube();
shambhala::Mesh *createScreen();
shambhala::Model *modelCreateSkyBox(const simple_vector<shambhala::TextureResource *> &textures);

const char *stacked(GLuint *parameters);
} // namespace util
} // namespace shambhala
