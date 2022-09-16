#include "util.hpp"
#include "ext/resource.hpp"
#include "shambhala.hpp"
#include "simple_vector.hpp"
#include "standard.hpp"

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

Mesh *util::meshCreateCube() {
  static Mesh *result = nullptr;
  if (result == nullptr) {
    result = shambhala::createMesh();
    result->meshLayout = getPrimitiveLayout();
    result->vertexBuffer = util::createCube();
  }
  return result;
}

static Program *skyboxProgram = nullptr;
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
