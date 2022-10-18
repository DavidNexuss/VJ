#pragma once
#include "worldmat.hpp"
#include <ext.hpp>
#include <shambhala.hpp>
#include <standard.hpp>

namespace shambhala {
namespace util {
simple_vector<uint8_t> createCube();
shambhala::Shader *createScreenVertexShader();
shambhala::Shader *createRegularVertexShader();
shambhala::Shader *createEmptyFragmentShader();
shambhala::Shader *createPassThroughShader();
shambhala::Program *createScreenProgram(IResource *fragmentShader);
shambhala::Program *createDepthOnlyProgram();
shambhala::Program *createRegularShaderProgram(IResource *fragmentShader);
shambhala::Program *createPassthroughEffect();
shambhala::Program *createBasicColored();
shambhala::Mesh *meshCreateCube();
shambhala::Mesh *createScreen();
shambhala::Mesh *createTexturedQuad();
shambhala::Model *
modelCreateSkyBox(const simple_vector<shambhala::TextureResource *> &textures);
shambhala::RenderCamera *createPassThroughCamera(RenderCamera *input);
int doSelectionPass(ModelList *models);

glm::mat4 translate(float x, float y, float z);
glm::mat4 rotate(float x, float y, float z, float angle);
glm::mat4 scale(float x, float y, float z);
glm::mat4 scale(float s);

void renderLine(glm::vec3 start, glm::vec3 end,
                glm::vec3 color = glm::vec3(1.0));
void renderLine(glm::vec3 start, glm::vec3 end, Material *material);
void renderPoint(glm::vec3 start, glm::vec3 color = glm::vec3(1.0));
void renderPlaneGrid(glm::vec3 x, glm::vec3 y, glm::vec3 origin,
                     glm::vec4 color = glm::vec4(1.0),
                     glm::vec2 size = glm::vec2(50.0f));
const char *stacked(GLuint *parameters);
} // namespace util
} // namespace shambhala
