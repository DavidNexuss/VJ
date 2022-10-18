#include "util.hpp"
#include "ext/resource.hpp"
#include "shambhala.hpp"
#include "simple_vector.hpp"
#include "standard.hpp"
#include <glm/ext/matrix_transform.hpp>
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
const static float screen_mesh[] = {-1.0, 1.0,  0.0, 1.0,  1.0,  0.0,
                                    -1.0, -1.0, 0.0, 1.0,  1.0,  0.0,
                                    1.0,  -1.0, 0.0, -1.0, -1.0, 0.0};

static const int screen_mesh_size = sizeof(screen_mesh) / sizeof(float);

static const float quad_mesh[] = {0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0,
                                  1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0,
                                  1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0};
static const int quad_mesh_size = sizeof(quad_mesh) / sizeof(float);

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

static vector<VertexAttribute> screenLayout = {{Standard::aPosition, 3}};

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

Mesh *util::createTexturedQuad() {
  static Mesh *result = nullptr;
  if (result == nullptr) {
    result = shambhala::createMesh();
    result->vbo = shambhala::createVertexBuffer();
    result->vbo->vertexBuffer = {quad_mesh, quad_mesh_size};
    result->vbo->attributes = {{Standard::aPosition, 2}, {Standard::aUV, 2}};
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

Shader *util::createScreenVertexShader() {
  IResource *resource = resource::createFromNullTerminatedString(
      screenVertexShader, "internal:screen_vertex_shader");
  return loader::loadShader(resource);
}
Shader *util::createEmptyFragmentShader() {
  IResource *file = resource::createFromNullTerminatedString(
      emptyFragShader, "internal:empty_frag_shader");
  return loader::loadShader(file);
}
Shader *util::createRegularVertexShader() {
  IResource *file = resource::createFromNullTerminatedString(
      regularVertexShader, "internal:regular_vert_shader");
  return loader::loadShader(file);
}
Shader *util::createPassThroughShader() {
  IResource *file = resource::createFromNullTerminatedString(
      passShader, "internal:pass_frag_effect");
  return loader::loadShader(file);
}

Program *util::createScreenProgram(IResource *resource) {
  Program *result = shambhala::createProgram();
  result->shaders[VERTEX_SHADER] = createScreenVertexShader();
  result->shaders[FRAGMENT_SHADER] = loader::loadShader(resource);
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
  result->shaders[FRAGMENT_SHADER] = loader::loadShader(fragmentShader);
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

Program *util::createBasicColored() {
  static Program *basic = shambhala::loader::loadProgram(
      "programs/misc/probe.fs", "programs/regular.vs");
  return basic;
}

static Program *getSkyboxProgram() {
  static Program *skyProgram = nullptr;
  if (skyProgram != nullptr)
    return skyProgram;

  skyProgram =
      loader::loadProgram("programs/cubemap.frag", "programs/cubemap.vert");
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

glm::mat4 util::translate(float x, float y, float z) {
  return glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
}

glm::mat4 util::rotate(float x, float y, float z, float angle) {
  return glm::rotate(glm::mat4(1.0f), angle, glm::vec3(x, y, z));
}

glm::mat4 util::scale(float x, float y, float z) {
  return glm::scale(glm::mat4(1.0f), glm::vec3(x, y, z));
}

glm::mat4 util::scale(float s) { return util::scale(s, s, s); }

void util::renderLine(glm::vec3 start, glm::vec3 end, glm::vec3 color) {
  static Material *mat = shambhala::createMaterial();
  mat->set("uColor", glm::vec4(color, 1.0));
  renderLine(start, end, mat);
}

void util::renderLine(glm::vec3 start, glm::vec3 end, Material *material) {

  static Model *renderModel = nullptr;
  static Program *probe = shambhala::loader::loadProgram(
      "programs/misc/probe.fs", "programs/regular.vs");

  if (renderModel == nullptr) {
    static float vertex_data[] = {0.0, 0.0, 0.0, 0.0, 0.0, 1.0};

    renderModel = shambhala::createModel();
    renderModel->program = probe;
    renderModel->node = shambhala::createNode();
    renderModel->mesh = shambhala::createMesh();
    renderModel->mesh->vbo = shambhala::createVertexBuffer();
    renderModel->mesh->vbo->vertexBuffer = {vertex_data, 6};
    renderModel->mesh->vbo->attributes = {{Standard::aPosition, 3}};
    renderModel->renderMode = GL_LINE_STRIP;
    renderModel->polygonMode = GL_LINE;
    renderModel->lineWidth = 15;
  }

  glm::vec3 offset = end - start;
  glm::mat4 transform = glm::mat4(1.0f);
  transform[2] = glm::vec4(offset, 0.0);
  transform[3] = glm::vec4(start, 1.0);

  renderModel->node->setTransformMatrix(transform);
  renderModel->material = material;
  renderModel->draw();
}

void util::renderPoint(glm::vec3 start, glm::vec3 color) {

  static Model *renderModel = nullptr;
  static Program *probe = shambhala::loader::loadProgram(
      "programs/misc/probe.fs", "programs/regular.vs");

  if (renderModel == nullptr) {
    static float vertex_data[] = {0.0, 0.0, 0.0};

    renderModel = shambhala::createModel();
    renderModel->program = probe;
    renderModel->node = shambhala::createNode();
    renderModel->mesh = shambhala::createMesh();
    renderModel->mesh->vbo = shambhala::createVertexBuffer();
    renderModel->mesh->vbo->vertexBuffer = {vertex_data, 6};
    renderModel->mesh->vbo->attributes = {{Standard::aPosition, 3}};
    renderModel->renderMode = GL_LINE_STRIP;
    renderModel->polygonMode = GL_POINT;
    renderModel->lineWidth = 15;
  }

  glm::mat4 transform = glm::mat4(1.0f);
  transform[3] = glm::vec4(start, 1.0);

  renderModel->node->setTransformMatrix(transform);
  renderModel->material = shambhala::createMaterial();
  renderModel->material->set("uColor", glm::vec4(color, 1.0));
  renderModel->draw();
}

int util::doSelectionPass(ModelList *models) {
  static Program *selection = shambhala::loader::loadProgram(
      "programs/select_pass.fs", "programs/regular.vs");
  static FrameBuffer *selectionBuffer = nullptr;
  static Material *selectionMaterial = shambhala::createMaterial();

  if (selectionBuffer == nullptr) {
    selectionBuffer = shambhala::createFramebuffer();
    selectionBuffer->setConfiguration(USE_RENDER_BUFFER | USE_DEPTH);
    selectionBuffer->addChannel({GL_R8, GL_RED, GL_UNSIGNED_BYTE});
  }

  glDisable(GL_MULTISAMPLE);
  selectionBuffer->begin(viewport()->screenWidth, viewport()->screenHeight);

  device::useProgram(selection);

  const std::vector<int> &order = models->getRenderOrder();
  for (int i = 0; i < order.size(); i++) {
    Model *model = models->get(order[i]);
    if (model->hint_selectionpass && model->isEnabled()) {

      selectionMaterial->set("uModelID", model->hint_modelid);
      device::useMesh(model->mesh);
      device::useMaterial(model->node);
      device::useMaterial(selectionMaterial);
      device::drawCall();
    }
  }

  char id;
  glReadPixels(viewport()->xpos, viewport()->screenHeight - viewport()->ypos, 1,
               1, GL_RED, GL_UNSIGNED_BYTE, &id);
  selectionBuffer->end();
  glEnable(GL_MULTISAMPLE);
  return id;
}

void util::renderPlaneGrid(glm::vec3 x, glm::vec3 y, glm::vec3 origin,
                           glm::vec4 color, glm::vec2 size) {
  static Model *planeModel = nullptr;
  if (planeModel == nullptr) {
    planeModel = shambhala::createModel();
    planeModel->program =
        loader::loadProgram("programs/grid.fs", "programs/regular.vs");
    planeModel->material = shambhala::createMaterial();
    planeModel->material->set("uColor", color);
    planeModel->mesh = createScreen();
    planeModel->node = shambhala::createNode();
  }
  glm::mat4 transform = planeModel->node->transformMatrix;
  transform[0] = glm::vec4(x * size.x, 0.0);
  transform[1] = glm::vec4(y * size.y, 0.0);
  transform[2] = glm::vec4(glm::normalize(glm::cross(x, y)), 0.0);
  transform[3] = glm::vec4(origin, 1.0);
  planeModel->node->setTransformMatrix(transform);
  planeModel->draw();
}
