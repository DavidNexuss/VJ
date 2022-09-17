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

using namespace shambhala;

simple_vector<uint8_t> util::createCube() {
  return {cube_mesh, cube_mesh_size};
}

MeshLayout *getPrimitiveLayout() {
  static MeshLayout *primitiveLayout = nullptr;
  if (primitiveLayout == nullptr) {
    primitiveLayout = shambhala::createMeshLayout();
    primitiveLayout->attributes = {
        {Standard::aPosition, 3}, {Standard::aNormal, 3}, {Standard::aUV, 2}};
  }
  return primitiveLayout;
}

MeshLayout *getScreenLayout() {
  static MeshLayout *primitiveLayout = nullptr;
  if (primitiveLayout == nullptr) {
    primitiveLayout = shambhala::createMeshLayout();
    primitiveLayout->attributes = {{Standard::aPosition, 2}};
  }
  return primitiveLayout;
}

Mesh *util::meshCreateCube() {
  static Mesh *result = nullptr;
  if (result == nullptr) {
    result = shambhala::createMesh();
    result->meshLayout = getPrimitiveLayout();
    result->vertexBuffer = util::createCube();
    result->invertedFaces = true;
  }
  return result;
}

Mesh *util::createScreen() {
  static Mesh *result = nullptr;
  if (result == nullptr) {
    result = shambhala::createMesh();
    result->meshLayout = getScreenLayout();
    result->vertexBuffer = {screen_mesh, screen_mesh_size};
    result->invertedFaces = true;
  }
  return result;
}

struct StaticMemoryResource : public IResource {
  io_buffer buffer;
  io_buffer *read() override { return &buffer; }
};

StaticMemoryResource *createFromNullTerminatedString(const char *data,
                                                     const char *resourcename) {
  StaticMemoryResource *resource = new StaticMemoryResource;
  resource->buffer = {(uint8_t *)data, Standard::resourceNullTerminated};
  resource->resourcename = resourcename;
  return resource;
}

Shader util::createScreenVertexShader() {
  Shader shader;
  StaticMemoryResource *res = new StaticMemoryResource;
  res->buffer = {(uint8_t *)screenVertexShader,
                 Standard::resourceNullTerminated};
  res->resourcename = "internal:screen_vertex_shader";
  shader.file = res;
  return shader;
}
Shader util::createEmptyFragmentShader() {
  Shader shader;
  shader.file = createFromNullTerminatedString(emptyFragShader,
                                               "internal:empty_frag_shader");
  return shader;
}
Shader util::createRegularVertexShader() {
  Shader shader;
  shader.file = createFromNullTerminatedString(regularVertexShader,
                                               "internal:regular_vert_shader");
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

static Program *getSkyboxProgram() {
  static Program *skyProgram = nullptr;
  if (skyProgram != nullptr)
    return skyProgram;

  skyProgram = createProgram();
  skyProgram->shaders[FRAGMENT_SHADER].file =
      resource::ioMemoryFile("materials/cubemap.frag");
  skyProgram->shaders[VERTEX_SHADER].file =
      resource::ioMemoryFile("materials/cubemap.vert");

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

const char *util::stacked(GLuint *array) {
  static char buffer[4096] = {0};
  int index = 0;
  while (*array)
    index += sprintf(&buffer[index], "%d ", *array++);

  return buffer;
}
