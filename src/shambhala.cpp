#include "shambhala.hpp"
#include "simple_vector.hpp"
#include "standard.hpp"
#include <algorithm>
#include <stb_image.h>
#include <unordered_map>
#include <vector>

#define REGISTER_UNIFORM_FLUSH()                                               \
  do {                                                                         \
  } while (0)

using namespace shambhala;

//---------------------[BEGIN NODE]

Node::Node() { clean = false; }
void Node::removeChildNode(Node *childNode) {}

void Node::setDirty() {
  clean = false;
  for (int i = 0; i < children.size(); i++) {
    children[i]->setDirty();
  }
}
void Node::addChildNode(Node *childNode) {
  children.push(childNode);
  childNode->setDirty();
}

void Node::setTransformMatrix(const glm::mat4 &newVal) {
  transformMatrix = newVal;
  setDirty();
}
const glm::mat4 &Node::getTransformMatrix() const { return transformMatrix; }

const glm::mat4 &Node::getCombinedMatrix() const {
  if (clean)
    return combinedMatrix;
  clean = true;
  if (parentNode != nullptr)
    return combinedMatrix = parentNode->getCombinedMatrix() * transformMatrix;
  else
    return combinedMatrix = transformMatrix;
}

//---------------------[END NODE]
//-----------------------[BEGIN UNIFORM]
bool Uniform::bind(GLuint glUniformID) const {

  switch (type) {
  case UniformType::VEC2:
    glUniform2fv(glUniformID, count, &VEC2[0]);
    break;
  case UniformType::VEC3:
    glUniform3fv(glUniformID, count, &VEC3[0]);
    break;
  case UniformType::VEC3PTR:
    glUniform3fv(glUniformID, count, &(*VEC3PTR)[0]);
    break;
  case UniformType::VEC4:
    glUniform4fv(glUniformID, count, &VEC4[0]);
    break;
  case UniformType::MAT2:
    glUniformMatrix2fv(glUniformID, count, false, &MAT2[0][0]);
    break;
  case UniformType::MAT3:
    glUniformMatrix3fv(glUniformID, count, false, &MAT3[0][0]);
    break;
  case UniformType::MAT4:
    glUniformMatrix4fv(glUniformID, count, false, &MAT4[0][0]);
    break;
  case UniformType::FLOAT:
    glUniform1f(glUniformID, FLOAT);
    break;
  case UniformType::BOOL:
  case UniformType::INT:
    glUniform1i(glUniformID, INT);
    break;
  case UniformType::SAMPLER2D:
    device::useTexture(SAMPLER2D);
    glUniform1i(glUniformID, SAMPLER2D.unit);
    break;
  case UniformType::DYNAMIC_TEXTURE:
    device::useTexture(DYNAMIC_TEXTURE.sourceTexture);
    glUniform1d(glUniformID, DYNAMIC_TEXTURE.unit);
    break;
  case UniformType::BINDLESS_TEXTURE:
    glUniformHandleui64ARB(glUniformID, BINDLESS_TEXTURE);
    break;
  }
  if (type != UniformType::SAMPLER2D)
    dirty = false;
  REGISTER_UNIFORM_FLUSH();
  return true;
}
//--------------------------[END UNIFORM]
//--------------------------[BEGIN MESHLAYOUT]
int MeshLayout::vertexSize() const {
  if (_vertexsize != -1)
    return _vertexsize;
  int vs = 0;
  for (int i = 0; i < attributes.size(); i++) {
    vs += attributes[i].size;
  }
  return _vertexsize = vs * sizeof(float);
}
//--------------------------[END MESHLAYOUT]

//--------------------------[BEGIN UPLOAD]
void device::uploadTexture(GLenum target, unsigned char *texturebuffer,
                           int width, int height, int components) {

  const static int internalFormat[] = {GL_RED, GL_RG8, GL_RGB8, GL_RGBA8};
  const static int externalFormat[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat[components - 1], width, height,
               0, externalFormat[components - 1], GL_UNSIGNED_BYTE,
               texturebuffer);

  glGenerateMipmap(GL_TEXTURE_2D);
}

void device::uploadDepthTexture(GLuint texture, int width, int height) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}
void device::uploadStencilTexture(GLuint texture, int width, int height) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}
//--------------------------[END UPLOAD]
void device::configureRenderBuffer(GLenum mode, int width, int height) {
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
}

//--------------------------[BEGIN COMPILE]

GLuint device::compileShader(const char *data, GLenum type) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &data, NULL);
  glCompileShader(shader);
  return shader;
}

GLuint device::compileProgram(GLuint *shaders) {
  GLuint program = glCreateProgram();
  GLuint *shaderPtr = shaders;
  while (*shaderPtr) {
    glAttachShader(program, *shaderPtr);
  }
  glLinkProgram(program);
  shaderPtr = shaders;
  while (*shaderPtr) {
    glDetachShader(program, *shaderPtr++);
  }
  return program;
}
//--------------------------[END COMPILE]

//--------------------------[BEGIN CREATE]

GLuint device::createVAO(
    const simple_vector<MeshLayout::MeshLayoutAttribute> &attributes) {
  GLuint vao;
  glGenVertexArrays(1, &vao);
  device::bindVao(vao);

  int vertexSize = 0;
  for (int i = 0; i < attributes.size(); i++) {
    vertexSize += attributes[i].size;
  }
  vertexSize *= sizeof(float);
  int offset = 0;
  for (int i = 0; i < attributes.size(); i++) {
    glVertexAttribPointer(attributes[i].index, attributes[i].size, GL_FLOAT,
                          GL_FALSE, vertexSize,
                          (void *)(offset * sizeof(float)));
    glEnableVertexAttribArray(attributes[i].index);
    offset += attributes[i].size;
  }
  return vao;
}
GLuint device::createVBO(const simple_vector<uint8_t> &vertexBuffer,
                         GLuint *vbo) {
  if (*vbo == -1) {
    glGenBuffers(1, vbo);
  }
  device::bindVbo(*vbo);
  glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size(), vertexBuffer.data(),
               GL_STATIC_DRAW);
  return *vbo;
}

GLuint device::createEBO(const simple_vector<unsigned short> &indexBuffer,
                         GLuint *ebo) {
  if (*ebo == -1) {
    glGenBuffers(1, ebo);
  }
  device::bindEbo(*ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               indexBuffer.size() * sizeof(unsigned short), indexBuffer.data(),
               GL_STATIC_DRAW);
  return *ebo;
}

GLuint device::createRenderBuffer() {
  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  return rbo;
}

GLuint device::createFramebuffer() {
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  return fbo;
}
GLuint device::createTexture(bool filter) {
  GLuint textureId;
  glGenTextures(1, &textureId);
  device::bindTexture(textureId, GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if (filter) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  return textureId;
}

GLuint device::createCubemap() {

  GLuint texId;
  glGenTextures(1, &texId);
  device::bindTexture(texId, GL_TEXTURE_CUBE_MAP);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  return texId;
}
//-------------------------[END CREATE]
//-------------------------[BEGIN BIND]

struct BindState {};
static BindState g_bindState;

void device::bindVao(GLuint vao) { glBindVertexArray(vao); }
void device::bindVbo(GLuint vbo) { glBindBuffer(GL_ARRAY_BUFFER, vbo); }
void device::bindEbo(GLuint ebo) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); }
void device::bindRenderBuffer(GLuint fbo) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}
//-------------------------[END BIND]

//-------------------------[BEGIN DISPOSE]
void device::disposeProgram(GLuint program) { glDeleteProgram(program); }
void device::disposeShader(GLuint shader) { glDeleteShader(shader); }
//-------------------------[END DISPOSE]
//---------------------------[BEGIN DEVICE USE]

struct UseState {

  // Current use state
  Program *currentProgram;
  Mesh *currentMesh = nullptr;
  MeshLayout *currentMeshLayout = nullptr;
  ModelConfiguration currentModelConfiguration;
  bool hasModelConfiguration = false;

  // Culling information
  bool needsFaceCullUpdate = false;
  bool lastcullFrontFace = false;
  bool invertedFaces = false;

  void bindUniforms(Material *material) {
    for (auto &uniform : material->uniforms) {
      uniform.second.bind(currentProgram->getUniform(uniform.first));
    }
  }
};
static UseState guseState;

void device::useMeshLayout(MeshLayout *layout) {
  if (layout->vao == -1)
    layout->vao = device::createVAO(layout->attributes);
  device::bindVao(layout->vao);
}
void device::useMesh(Mesh *mesh) {
  device::useMeshLayout(mesh->meshLayout);
  if (mesh->indexBuffer.size() && (mesh->needsEBOUpdate || mesh->ebo == -1)) {
    device::createEBO(mesh->indexBuffer, &mesh->vbo);
    mesh->needsEBOUpdate = false;
  }
  if (mesh->vertexBuffer.size() && (mesh->needsVBOUpdate || mesh->vbo == -1)) {
    device::createVBO(mesh->vertexBuffer, &mesh->ebo);
    mesh->needsVBOUpdate = false;
  }
  device::bindVbo(mesh->vbo);
  device::bindEbo(mesh->ebo);
}

void device::useTexture(Texture *texture) {

  if (texture->_textureID == -1)
    texture->_textureID = texture->isCubemap ? device::createCubemap()
                                             : device::createTexture(true);

  if (texture->needsTextureUpdate) {
    GLenum target = texture->isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    device::bindTexture(texture->_textureID, target);
    for (int i = 0; i < texture->textureData.size(); i++) {
      device::uploadTexture(target, texture->textureData[i]->textureBuffer,
                            texture->textureData[i]->width,
                            texture->textureData[i]->height,
                            texture->textureData[i]->components);
    }
  }
}

// TODO:Improve quality, also check if caching is any useful
void device::useModelConfiguration(ModelConfiguration *configuration) {

  if (configuration->cullFrontFace != guseState.lastcullFrontFace) {
    guseState.lastcullFrontFace = configuration->cullFrontFace;
    guseState.needsFaceCullUpdate = true;
  }

  if (!guseState.hasModelConfiguration ||
      (configuration->pointSize !=
       guseState.currentModelConfiguration.pointSize)) {
    glPointSize(configuration->pointSize);
  }

  if (!guseState.hasModelConfiguration ||
      (configuration->polygonMode !=
       guseState.currentModelConfiguration.polygonMode)) {
    glPolygonMode(GL_FRONT_AND_BACK, configuration->polygonMode);
  }
  guseState.currentModelConfiguration = *configuration;
  guseState.hasModelConfiguration = true;
}

void device::useMaterial(Material *material) {
  guseState.bindUniforms(material);
}
//---------------------[END DEVICE USE]

// --------------------[BEGIN FRAMEBUFFER]

void FrameBuffer::initialize() {
  _framebuffer = device::createFramebuffer();
  device::bindFrameBuffer(_framebuffer);
  glGenTextures(colorAttachments.size(), &colorAttachments[0]);
  for (int i = 0; i < colorAttachments.size(); i++) {
    device::bindTexture(colorAttachments[i], GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, attachmentsDefinition[i].internalFormat,
                 bufferWidth, bufferHeight, 0,
                 attachmentsDefinition[i].externalFormat,
                 attachmentsDefinition[i].type, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, i + GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, colorAttachments[i], 0);
  }

  if (combinedDepthStencil()) {
    if (configuration & USE_RENDER_BUFFER) {
      stencilDepthBuffer = createDepthStencilBuffer();
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                GL_RENDERBUFFER, stencilDepthBuffer);
    } else {
      stencilDepthBuffer = device::createTexture(false);
      device::uploadDepthStencilTexture(stencilDepthBuffer, bufferWidth,
                                        bufferHeight);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                             GL_TEXTURE_2D, stencilDepthBuffer, 0);
    }
  } else {
    if (configuration & USE_DEPTH) {
      depthBuffer = device::createTexture(false);
      device::uploadDepthTexture(depthBuffer, bufferWidth, bufferHeight);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                             depthBuffer, 0);
    }

    if (configuration & USE_STENCIL) {
      stencilBuffer = device::createTexture(false);
      device::uploadStencilTexture(stencilBuffer, bufferWidth, bufferHeight);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                             GL_TEXTURE_2D, stencilBuffer, 0);
    }
  }

  if (colorAttachments.size() == 0) {
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  }

  // TODO: Makes sense to reorder color attachments, should look this up :D
  const static unsigned int buffers[16] = {
      GL_COLOR_ATTACHMENT0,  GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT3,
      GL_COLOR_ATTACHMENT4,  GL_COLOR_ATTACHMENT5,  GL_COLOR_ATTACHMENT6,
      GL_COLOR_ATTACHMENT7,  GL_COLOR_ATTACHMENT8,  GL_COLOR_ATTACHMENT9,
      GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11, GL_COLOR_ATTACHMENT12,
      GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15,
  };

  glDrawBuffers(colorAttachments.size(), buffers);
}

void FrameBuffer::dispose() {
  device::bindFrameBuffer(0);
  glDeleteTextures(colorAttachments.size(), colorAttachments.data());

  if (combinedDepthStencil()) {
    if (configuration & USE_RENDER_BUFFER)
      glDeleteRenderbuffers(1, &stencilDepthBuffer);
    else
      glDeleteTextures(1, &stencilDepthBuffer);
  } else {
    if (configuration & USE_DEPTH) {
      glDeleteTextures(1, &depthBuffer);
    }
    if (configuration & USE_STENCIL) {
      glDeleteTextures(1, &stencilBuffer);
    }
  }
  glDeleteFramebuffers(1, &_framebuffer);
}

void FrameBuffer::resize(int screenWidth, int screenHeight) {
  if (bufferWidth == screenWidth && bufferHeight == screenHeight)
    return;
  if (_framebuffer != -1)
    dispose();

  bufferWidth = screenWidth;
  bufferHeight = screenHeight;

  initialize();
  check();
}

void FrameBuffer::check() {
  bool error =
      glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE;
  if (error) {
  }
}

void FrameBuffer::begin(int screenWidth, int screenHeight) {
  resize(screenWidth, screenHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBuffer::end() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

//---------------------[FRAMEBUFFER END]
//---------------------[MODEL BEGIN]
bool Model::operator<(const Model &model) const {

  return zIndex < model.zIndex ||
         (zIndex == model.zIndex &&
          (program < model.program ||
           (program == model.program &&
            (material < model.material ||
             (material == model.material && mesh < model.mesh)))));
}
bool Model::ready() const { return program != nullptr && mesh != nullptr; }
void Model::draw() {
  device::useModelConfiguration(this);
  device::useMesh(mesh);
  device::useProgram(program);
  if (material != nullptr)
    device::useMaterial(material);
}
//---------------------[MODEL END]
//---------------------[BEGIN ENGINE]
struct Engine {
  std::vector<int> modelindices;
  simple_vector<Model *> models;
  bool shouldSort = false;

  const std::vector<int> &getRenderOrder() {
    if (shouldSort) {
      modelindices.resize(models.size());
      for (int i = 0; i < modelindices.size(); i++)
        modelindices[i] = i;
      std::sort(modelindices.begin(), modelindices.end(),
                [&](int lhs, int rhs) { return *models[lhs] < *models[rhs]; });
    }
    return modelindices;
  }
};

static Engine engine;
//---------------------[BEGIN ENGINECONFIGURATION]
static IViewport *_viewport;
void shambhala::createEngine(EngineParameters parameters) {
  _viewport = parameters.viewport;
}

IViewport *shambhala::viewport() { return _viewport; }
//---------------------[END ENGINECONFIGURATION]

//---------------------[BEGIN ENGINECREATE]

Texture *shambhala::createTexture() { return new Texture; }
Model *shambhala::createModel() { return new Model; }
Mesh *shambhala::createMesh() { return new Mesh; }
MeshLayout *shambhala::createMeshLayout() { return new MeshLayout; }
Program *shambhala::createProgram() { return new Program; }
FrameBuffer *shambhala::createFramebuffer() { return new FrameBuffer; }
Material *shambhala::createMaterial() { return new Material; }

//---------------------[END ENGINECREATE]

//---------------------[BEGIN ENGINEUPDATE]

void shambhala::buildSortPass() { engine.shouldSort = true; }
void shambhala::renderPass() {
  const std::vector<int> &renderOrder = engine.getRenderOrder();
  for (int i = 0; i < renderOrder.size(); i++) {
    Model *model = engine.models[renderOrder[i]];
    if (model->enabled) {
      model->draw();
    }
  }
}
//---------------------[END ENGINEUPDATE]
//---------------------[END ENGINE]
int main() {}
