#include "util.hpp"
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
  Mesh *result = shambhala::createMesh();
  result->meshLayout = getPrimitiveLayout();
  result->vertexBuffer = util::createCube();
  return result;
}

static Mesh *skyboxMesh = nullptr;
static Program *skyboxProgram = nullptr;

static MeshLayout *getSkyboxLayout() {
  static MeshLayout *skyboxLayout = nullptr;
  return skyboxLayout;
}
static Mesh *getSkyboxMesh() {
  static Mesh *skyboxMesh = nullptr;
  return skyboxMesh;
}
static Program *getSkyboxProgram() {
  static Program *skyProgram = nullptr;
  if (skyProgram != nullptr)
    return skyProgram;

  skyProgram = createProgram();
  skyProgram->shaders[FRAGMENT_SHADER].file =
      resource::ioMemoryFile("assets/materials");
  return skyProgram;
}

Model *util::modelCreateSkyBox(
    const simple_vector<shambhala::TextureResource *> &textures) {
  Model *result = shambhala::createModel();
  result->mesh = getSkyboxMesh();
  result->program = getSkyboxProgram();
  result->material = shambhala::createMaterial();
  Texture *skyCubemap = shambhala::createTexture();
  for (int i = 0; i < textures.size(); i++) {
    skyCubemap->addTextureResource(textures[i]);
  }
  DynamicTexture dyn;
  dyn.sourceTexture = skyCubemap;
  dyn.mode = GL_TEXTURE_CUBE_MAP;
  dyn.unit = Standard::tSkyBox;
  result->material->set("skyTexture", dyn);
  return result;
}

const char *util::stacked(GLuint *array) {
  static char buffer[4096] = {0};
  int index = 0;
  while (*array)
    index += sprintf(&buffer[index], "%d ", *array++);

  return buffer;
}
