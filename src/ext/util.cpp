#include "util.hpp"
#include "ext/resource.hpp"
#include "shambhala.hpp"
#include "simple_vector.hpp"
#include "standard.hpp"
#include <unistd.h>

static const GLfloat cube_mesh[] = {

    // -Z
    -1.0, 1.0, -1.0, 0.0, 1.0, 0.0, 0.0, 1.0, -1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 1.0, -1.0, 0.0, 0.0, 1.0, 1.0, 1.0,

    1.0, -1.0, -1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, -1.0, 0.0, 0.0, 1.0,
    1.0, 1.0, -1.0, -1.0, -1.0, 1.0, 0.0, 0.0, 0.0, 0.0,

    // +Z

    -1.0, -1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, -1.0, 1.0, 1.0, 0.0, 1.0, 0.0,
    1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,

    1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, -1.0, 1.0, 0.0, 1.0, 0.0, 0.0,
    1.0, -1.0, -1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0,

    // +Y

    -1.0, 1.0, -1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, -1.0, 0.0, 0.0, 1.0,
    0.0, 0.0, -1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0,

    -1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0,

    // -Y

    1.0, -1.0, -1.0, 0.0, 1.0, 0.0, 1.0, 1.0, -1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
    1.0, 0.0, -1.0, -1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0,

    1.0, -1.0, -1.0, 0.0, 1.0, 0.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, -1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0,

    // +X

    1.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, -1.0, -1.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, -1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0,

    1.0, -1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0,
    0.0, 1.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0,

    // -X

    -1.0, -1.0, -1.0, 1.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 0.0, 1.0, 0.0,
    0.0, 0.0, -1.0, -1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0,

    -1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, -1.0, -1.0, 1.0, 1.0, 0.0, 0.0,
    1.0, 1.0, -1.0, 1.0, -1.0, 0.0, 1.0, 0.0, 0.0, 0.0

};

static const int cube_mesh_size = sizeof(cube_mesh) / sizeof(float);
const static float screen_mesh[] = {-1.0, 1.0, 1.0, 1.0,  -1.0, -1.0,
                                    1.0,  1.0, 1.0, -1.0, -1.0, -1.0};

static const int screen_mesh_size = sizeof(screen_mesh) / sizeof(float);

static const char *screenVertexShader = " \
#version 330 core\n \
    layout(location = 0) in vec2 aPosition; \
 \
out vec2 uv; \
 \
void main() { \
  gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0); \
  uv = aPosition * 0.5 + 0.5; \
} ";

static const char *regularVertexShader = " \
#version 330 core\n \
layout(location = 0) in vec3 aVertex; \
uniform mat4 uProjectionMatrix; \
uniform mat4 uViewMatrix; \
uniform mat4 uTransformMatrix; \
 \
void main() { \
  gl_Position = uProjectionMatrix * uViewMatrix * uTransformMatrix * vec4(aVertex, 1.0); \
}";

static const char *emptyFragShader = " \
#version 330 core\n \
void main() { } \
";

static const char *passShader = " \
#version 330 core\n \
uniform sampler2D input; \
out vec3 color; \
in vec2 uv; \
void main() {  color = texture2D(input, uv).xyz; }\
";

using namespace shambhala;

simple_vector<uint8_t> util::createCube() {
  return {cube_mesh, cube_mesh_size};
}

static vector<VertexAttribute> primitiveLayout = {
    {Standard::aPosition, 3}, {Standard::aNormal, 3}, {Standard::aUV, 2}};

static vector<VertexAttribute> screenLayout = {{Standard::aPosition, 2}};

Mesh *util::meshCreateCube() {
  static Mesh *result = nullptr;
  if (result == nullptr) {
    result = shambhala::createMesh();
    result->vbo = shambhala::createVertexBuffer();
    result->vbo->vertexBuffer = util::createCube();
    result->vbo->attributes = primitiveLayout;
    result->invertedFaces = true;
  }
  return result;
}

Mesh *util::createScreen() {
  static Mesh *result = nullptr;
  if (result == nullptr) {
    result = shambhala::createMesh();
    result->vbo = shambhala::createVertexBuffer();
    result->vbo->attributes = screenLayout;
    result->vbo->vertexBuffer = {screen_mesh, screen_mesh_size};
    result->invertedFaces = true;
  }
  return result;
}

Shader util::createScreenVertexShader() {
  Shader shader;
  shader.file = resource::createFromNullTerminatedString(
      screenVertexShader, "internal:screen_vertex_shader");
  return shader;
}
Shader util::createEmptyFragmentShader() {
  Shader shader;
  shader.file = resource::createFromNullTerminatedString(
      emptyFragShader, "internal:empty_frag_shader");
  return shader;
}
Shader util::createRegularVertexShader() {
  Shader shader;
  shader.file = resource::createFromNullTerminatedString(
      regularVertexShader, "internal:regular_vert_shader");
  return shader;
}
Shader util::createPassThroughShader() {
  Shader shader;
  shader.file = resource::createFromNullTerminatedString(
      passShader, "internal:pass_frag_effect");
  return shader;
}

Program *util::createScreenProgram(IResource *resource) {
  Program *result = shambhala::createProgram();
  result->shaders[VERTEX_SHADER] = createScreenVertexShader();
  result->shaders[FRAGMENT_SHADER].file = resource;
  return result;
}

Program *util::createDepthOnlyProgram() {
  Program *result = shambhala::createProgram();
  result->shaders[VERTEX_SHADER] = util::createRegularVertexShader();
  result->shaders[FRAGMENT_SHADER] = util::createEmptyFragmentShader();
  return result;
}

Program *util::createRegularShaderProgram(IResource *fragmentShader) {
  Program *result = shambhala::createProgram();
  result->shaders[VERTEX_SHADER] = util::createRegularVertexShader();
  result->shaders[FRAGMENT_SHADER].file = fragmentShader;
  return result;
}

Program *util::createPassthroughEffect() {
  static Program *result = nullptr;
  if (result == nullptr) {
    result = shambhala::createProgram();
    result->shaders[FRAGMENT_SHADER] = util::createPassThroughShader();
    result->shaders[VERTEX_SHADER] = util::createScreenVertexShader();
  }
  return result;
}

static Program *getSkyboxProgram() {
  static Program *skyProgram = nullptr;
  if (skyProgram != nullptr)
    return skyProgram;

  skyProgram = createProgram();
  skyProgram->shaders[FRAGMENT_SHADER].file =
      resource::ioMemoryFile("programs/cubemap.frag");
  skyProgram->shaders[VERTEX_SHADER].file =
      resource::ioMemoryFile("programs/cubemap.vert");

  skyProgram->hint_skybox = true;

  return skyProgram;
}

Model *util::modelCreateSkyBox(
    const simple_vector<shambhala::TextureResource *> &textures) {
  Model *result = shambhala::createModel();
  result->mesh = meshCreateCube();
  result->program = getSkyboxProgram();
  result->material = shambhala::createMaterial();
  result->depthMask = true;
  result->cullFrontFace = true;
  result->zIndex = -1;

  Texture *skyCubemap = shambhala::createTexture();
  for (int i = 0; i < textures.size(); i++) {
    skyCubemap->addTextureResource(textures[i]);
  }
  skyCubemap->textureMode = GL_TEXTURE_CUBE_MAP;

  DynamicTexture dyn;
  dyn.sourceTexture = skyCubemap;
  dyn.unit = Standard::tSkyBox;
  result->material->set(Standard::uSkyBox, dyn);

  return result;
}

RenderCamera *util::createPassThroughCamera(RenderCamera *input) {
  RenderCamera *result = shambhala::createRenderCamera();
  result->addInput(input, 0, "input");
  result->postprocessProgram = createPassthroughEffect();
  return result;
}
const char *util::stacked(GLuint *array) {
  static char buffer[4096] = {0};
  int index = 0;
  while (*array)
    index += sprintf(&buffer[index], "%d ", *array++);

  return buffer;
}
