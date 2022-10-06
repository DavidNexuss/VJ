#pragma once
#include <ext.hpp>
#include <shambhala.hpp>
#include <standard.hpp>
namespace shambhala {
namespace util {
simple_vector<uint8_t> createCube();
shambhala::Shader createScreenVertexShader();
shambhala::Shader createRegularVertexShader();
shambhala::Shader createEmptyFragmentShader();
shambhala::Shader createPassThroughShader();
shambhala::Program *createScreenProgram(IResource *fragmentShader);
shambhala::Program *createDepthOnlyProgram();
shambhala::Program *createRegularShaderProgram(IResource *fragmentShader);
shambhala::Program *createPassthroughEffect();
shambhala::Mesh *meshCreateCube();
shambhala::Mesh *createScreen();
shambhala::Model *
modelCreateSkyBox(const simple_vector<shambhala::TextureResource *> &textures);

shambhala::RenderCamera *createPassThroughCamera(RenderCamera *input);

const char *stacked(GLuint *parameters);
} // namespace util
} // namespace shambhala
