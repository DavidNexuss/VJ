#pragma once
#include <shambhala.hpp>
#include <ext.hpp>
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


template <typename T> void debugRenderLoop(T &&function) {
  worldmats::Camera* debugCamera = new worldmats::DebugCamera;
  shambhala::setWorldMaterial(Standard::wCamera, debugCamera);
  do {
    function();
    viewport()->dispatchRenderEvents();
  } while (!shambhala::loop_shouldClose());
}

const char *stacked(GLuint *parameters);
} // namespace util
} // namespace shambhala
