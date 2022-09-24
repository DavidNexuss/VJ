#include "shambhala.hpp"
#include "adapters/log.hpp"
#include "core/core.hpp"
#include "simple_vector.hpp"
#include "standard.hpp"
#include <algorithm>
#include <ext.hpp>
#include <ext/util.hpp>
#include <memory>
#include <stbimage/stb_image.h>
#include <unordered_map>
#include <utility>
#include <vector>

#define REGISTER_UNIFORM_FLUSH()                                               \
  do {                                                                         \
  } while (0)

#define TEXTURE_CACHING 0

using namespace shambhala;

void glError(GLenum source, GLenum type, GLuint id, GLenum severity,
             GLsizei length, const GLchar *message, const void *userParam) {

  std::cerr << "[GL " << severity << "] " << message << std::endl;
  if (severity >= 37190) {
    // throw std::runtime_error{"error"};
  }
}

inline void clearFramebuffer() { glClearColor(0.0, 0.0, 0.0, 1.0); }
inline void clearDefault() { glClearColor(0.0, 0.0, 0.0, 1.0); }

struct Engine {
  EngineControllers controllers;
  ModelList *workingModelList;
  RenderCamera *currentRenderCamera;
  DeviceParameters gpu_params;
  GLuint vao = -1;

  const char *errorProgramFS = "programs/error.fs";
  const char *errorProgramVS = "programs/error.vs";

  Program *defaultProgram = nullptr;
  Material *defaultMaterial = nullptr;

  bool prepared = false;
  void init() {
    workingModelList = shambhala::createModelList();
    gpu_params = device::queryDeviceParameters();

    defaultProgram = loader::loadProgram(errorProgramFS, errorProgramVS);
    defaultMaterial = shambhala::createMaterial();
  }

  void prepareRender() {
    if (prepared)
      return;
    prepared = true;

    clearDefault();
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    vao = device::createVAO();
#ifdef DEBUG
    glDebugMessageCallback(glError, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
#endif
  }

  void cleanup() { loader::unloadProgram(defaultProgram); }
};

static Engine engine;
//---------------------[BEGIN NODE]

Node::Node() {
  clean = false;
  hasCustomBindFunction = true;
}
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
void Node::setParentNode(Node *parent) {
  parentNode = parent;
  setDirty();
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

void Node::bind(Program *activeProgram) {
  device::useUniform(Standard::uTransformMatrix, Uniform(getCombinedMatrix()));
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
    glUniform1i(glUniformID, SAMPLER2D.unit);
    device::useTexture(SAMPLER2D);
    break;
  case UniformType::DYNAMIC_TEXTURE:
    glUniform1i(glUniformID, DYNAMIC_TEXTURE.unit);
    device::useTexture(DYNAMIC_TEXTURE);
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
int VertexBuffer::vertexSize() const {
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
  glTexImage2D(target, 0, internalFormat[components - 1], width, height, 0,
               externalFormat[components - 1], GL_UNSIGNED_BYTE, texturebuffer);
}

void device::uploadDepthTexture(GLuint texture, int width, int height) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}
void device::uploadStencilTexture(GLuint texture, int width, int height) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}
void device::uploadDepthStencilTexture(GLuint texture, int width, int height) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0,
               GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
}
//--------------------------[END UPLOAD]
void device::configureRenderBuffer(GLenum mode, int width, int height) {
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
}

//--------------------------[BEGIN COMPILE]

GLuint device::getShaderType(int shaderType) {
  const static GLuint shaderTypes[] = {
      GL_FRAGMENT_SHADER, GL_VERTEX_SHADER, GL_GEOMETRY_SHADER,
      GL_TESS_EVALUATION_SHADER, GL_TESS_CONTROL_SHADER};
  return shaderTypes[shaderType];
}
GLuint device::compileShader(const char *data, GLenum type,
                             const char *resourcename) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &data, NULL);
  glCompileShader(shader);
  GLint compileStatus = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus == GL_FALSE) {
    LOG("[DEVICE] Error compiling shader %d: %s", shader, resourcename);
  } else {
    LOG("[DEVICE] Shader compiled successfully %d: %s", shader, resourcename);
  }
  return shader;
}

GLuint device::compileProgram(GLuint *shaders, GLint *status) {
  GLuint program = glCreateProgram();
  GLuint *shaderPtr = shaders;
  while (*shaderPtr) {
    glAttachShader(program, *shaderPtr++);
  }
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, status);
  GLint InfoLogLength;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
    glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    LOG("%s", &ProgramErrorMessage[0]);
  }
  if (*status == GL_FALSE) {
    LOG("[DEVICE] Program link error %d : %s", program, util::stacked(shaders));
  } else {
    LOG("[DEVICE] Program link success %d : %s", program,
        util::stacked(shaders));
  }

  shaderPtr = shaders;
  while (*shaderPtr) {
    glDetachShader(program, *shaderPtr++);
  }
  return program;
}
//--------------------------[END COMPILE]

//--------------------------[BEGIN CREATE]

GLuint device::createVAO() {
  GLuint vao;
  glGenVertexArrays(1, &vao);
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
//-------------------------[BEGIN GL]

struct BindState {
  GLuint boundTextures[Standard::maxTextureUnits] = {0};
  GLuint boundAttributes[100] = {0};
  GLuint currentProgram = -1;
  GLuint currentVbo = -1;
  GLuint currentEbo = -1;
  int activeTextureUnit = -1;

  bool cullFrontFace = false;
  void clearState() {}

  GLuint currentVao = -1;

  void printBindState() {}
};
static BindState gBindState;

void device::bindVao(GLuint vao) {
  if (gBindState.currentVao == vao)
    return;
  gBindState.currentVao = vao;
  glBindVertexArray(vao);
}
void device::bindVbo(GLuint vbo) {
  if (gBindState.currentVbo == vbo)
    return;
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  gBindState.currentVbo = vbo;
}
void device::bindEbo(GLuint ebo) {
  if (gBindState.currentEbo == ebo)
    return;
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  gBindState.currentEbo = ebo;
}
void device::bindRenderBuffer(GLuint rbo) {
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
}

void device::bindTexture(GLuint textureId, GLenum mode) {
  device::bindTexture(textureId, mode, Standard::tCreation);
}

void device::bindTexture(GLuint textureId, GLenum mode, int textureUnit) {

#if (TEXTURE_CACHING)

  if (gBindState.boundTextures[textureUnit] != textureId) {
    gBindState.boundTextures[textureUnit] = textureId;
    if (gBindState.activeTextureUnit != textureUnit) {
      glActiveTexture(textureUnit + GL_TEXTURE0);
      gBindState.activeTextureUnit = textureUnit;
    }
    glBindTexture(mode, textureId);
  }

#else

  glActiveTexture(textureUnit + GL_TEXTURE0);
  glBindTexture(mode, textureId);
#endif
}

void device::bindAttribute(GLuint vbo, int index, int size, int stride,
                           int offset, int divisor) {

  if (gBindState.boundAttributes[index] != vbo) {
    if (gBindState.boundAttributes[index] == 0)
      glEnableVertexAttribArray(index);
    gBindState.boundAttributes[index] = vbo;
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride,
                          (void *)(size_t(offset)));

    if (divisor != 0) {
      glVertexAttribDivisor(index, divisor);
    }
  }
}

void device::bindFrameBuffer(GLuint frameBuffer) {
  glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
}
void device::bindProgram(GLuint program) {
  if (gBindState.currentProgram == program)
    return;
  glUseProgram(program);
  gBindState.currentProgram = program;
}
GLuint device::getUniform(GLuint program, const char *name) {
  return glGetUniformLocation(program, name);
}

void device::cullFrontFace(bool frontFace) {
  if (frontFace != gBindState.cullFrontFace) {
    glCullFace(frontFace ? GL_FRONT : GL_BACK);
    gBindState.cullFrontFace = frontFace;
  }
}
//-------------------------[END GL]

//-------------------------[BEGIN DISPOSE]
void device::disposeProgram(GLuint program) { glDeleteProgram(program); }
void device::disposeShader(GLuint shader) { glDeleteShader(shader); }
//-------------------------[END DISPOSE]
//---------------------------[BEGIN DEVICE USE]

struct UseState {

  // Current use state
  Program *currentProgram = nullptr;
  Mesh *currentMesh = nullptr;
  ModelList *currentModelList = nullptr;
  VertexBuffer *currentVertexBuffer = nullptr;
  IndexBuffer *currentIndexBuffer = nullptr;
  ModelConfiguration currentModelConfiguration;

  bool hasModelConfiguration = false;
  bool ignoreProgramUse = false;

  // Culling information
  bool modelCullFrontFace = false;
  bool meshCullFrontFace = false;

  std::unordered_map<int, Material *> worldMaterials;

  void clearState() {
    currentProgram = nullptr;
    currentMesh = nullptr;
    hasModelConfiguration = false;
  }

  bool isFrontCulled() { return meshCullFrontFace ^ modelCullFrontFace; }
};
static UseState guseState;

void device::useProgram(Program *program) {
  if (guseState.currentProgram == program || guseState.ignoreProgramUse)
    return;

  SoftCheck(program != nullptr,
            LOG("[Warning] Trying to use a null program!", 0););

  if (program == nullptr || program->errored) {
    device::useProgram(engine.defaultProgram);
    return;
  }
  // Check program compilation with shaders
  bool programUpdate = program->shaderProgram == -1;
  GLuint shaders[SHADER_TYPE_COUNT + 1] = {0};
  int currentShader = 0;
  for (int i = 0; i < SHADER_TYPE_COUNT; i++) {
    if (program->shaders[i].file != nullptr) {
      bool shaderUpdate = program->shaders[i].file->claim();
      programUpdate |= shaderUpdate;
      if (shaderUpdate) {
        program->shaders[i].shader = device::compileShader(
            (const char *)program->shaders[i].file->read()->data,
            device::getShaderType(i), program->shaders[i].file->resourcename);
      }

      shaders[currentShader++] = program->shaders[i].shader;
    }
  }

  if (programUpdate) {
    GLint status;
    program->shaderProgram = device::compileProgram(shaders, &status);
    program->errored = !status;
  }

  device::bindProgram(program->shaderProgram);
  guseState.currentProgram = program;

  // Bind worldMaterials
  for (auto &worldmat : guseState.worldMaterials) {
    useMaterial(worldmat.second);
  }
}

void device::ignoreProgramBinding(bool ignore) {
  guseState.ignoreProgramUse = ignore;
}

void device::useIndexBuffer(IndexBuffer *indexBuffer) {

  if (indexBuffer == guseState.currentIndexBuffer)
    return;

  guseState.currentIndexBuffer = indexBuffer;
  if (indexBuffer->indexBuffer.size() &&
      (indexBuffer->updateData || indexBuffer->ebo == -1)) {

    device::createEBO(indexBuffer->indexBuffer, &indexBuffer->ebo);
    indexBuffer->updateData = false;
  }

  device::bindEbo(indexBuffer->ebo);
}

void device::useVertexBuffer(VertexBuffer *vertexbuffer) {

  if (vertexbuffer == guseState.currentVertexBuffer)
    return;

  guseState.currentVertexBuffer = vertexbuffer;
  if (vertexbuffer->vertexBuffer.size() && (vertexbuffer->vbo == -1)) {
    device::createVBO(vertexbuffer->vertexBuffer, &vertexbuffer->vbo);
    vertexbuffer->updateData = false;
  }

  if (vertexbuffer->updateData) {
    device::bindVbo(vertexbuffer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexbuffer->vertexBuffer.size(),
                    vertexbuffer->vertexBuffer.data());
  }

  device::bindVbo(vertexbuffer->vbo);

  SoftCheck(vertexbuffer->attributes.size() != 0, {
    LOG("[Warning] Vertexbuffer with not attributes! %d",
        (int)vertexbuffer->attributes.size());
  });
  int offset = 0;
  int stride = vertexbuffer->vertexSize();
  for (int i = 0; i < vertexbuffer->attributes.size(); i++) {
    int index = vertexbuffer->attributes[i].index;
    int divisor = vertexbuffer->attributes[i].attributeDivisor;
    int size = vertexbuffer->attributes[i].size;

    device::bindAttribute(vertexbuffer->vbo, index, size, stride,
                          offset * sizeof(float), divisor);
    offset += vertexbuffer->attributes[i].size;
  }
}
void device::useMesh(Mesh *mesh) {
  SoftCheck(mesh != nullptr, log()->log("[Warning] Null mesh use"););
  if (guseState.currentMesh == mesh)
    return;
  device::useVertexBuffer(mesh->vbo);
  if (mesh->ebo)
    device::useIndexBuffer(mesh->ebo);

  guseState.currentMesh = mesh;
  guseState.meshCullFrontFace = mesh->invertedFaces;
}

void device::useTexture(UTexture texture) {
  device::bindTexture(texture.texID, texture.mode, texture.unit);
}

void device::useTexture(Texture *texture) {

  SoftCheck(texture != nullptr, log()->log("[Warning] Null texture use", 1););
  if (texture->_textureID == -1)
    texture->_textureID = texture->textureMode == GL_TEXTURE_CUBE_MAP
                              ? device::createCubemap()
                              : device::createTexture(true);

  if (texture->needsUpdate()) {
    GLenum target = texture->textureMode;
    device::bindTexture(texture->_textureID, target);
    for (int i = 0; i < texture->textureData.size(); i++) {
      if (texture->textureData[i]->claim()) {
        GLenum target = texture->textureMode == GL_TEXTURE_CUBE_MAP
                            ? (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i)
                            : GL_TEXTURE_2D;
        device::uploadTexture(target, texture->textureData[i]->textureBuffer,
                              texture->textureData[i]->width,
                              texture->textureData[i]->height,
                              texture->textureData[i]->components);
      }
    }
    glGenerateMipmap(target);
  }
}

void device::useTexture(DynamicTexture texture) {
  device::useTexture(texture.sourceTexture);
  UTexture text{};
  text.mode = texture.sourceTexture->textureMode;
  text.texID = texture.sourceTexture->_textureID;
  text.unit = texture.unit;
  device::useTexture(text);
}

// TODO:Improve quality, also check if caching is any useful
void device::useModelConfiguration(ModelConfiguration *configuration) {

  guseState.modelCullFrontFace = configuration->cullFrontFace;

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

void device::useUniform(const char *name, const Uniform &value) {
  GLuint uniformId =
      device::getUniform(guseState.currentProgram->shaderProgram, name);

  // SoftCheck(uniformId != -1, LOG("[Warning] Attempted to use an invalid
  // uniform %s", name););
  if (uniformId != -1)
    value.bind(uniformId);
}

void device::useMaterial(Material *material) {
  if (material == nullptr)
    return;
  if (material->currentFrame != engine.currentRenderCamera->currentFrame) {
    material->update(viewport()->deltaTime);
  }

  material->currentFrame = engine.currentRenderCamera->currentFrame;
  SoftCheck(material != nullptr, log()->log("[Warning] Null material use"););

  for (auto &uniform : material->uniforms) {
    useUniform(uniform.first.c_str(), uniform.second);
  }
  if (material->hasCustomBindFunction) {
    material->bind(guseState.currentProgram);
  }

  for (int i = 0; i < material->childMaterials.size(); i++) {
    device::useMaterial(material->childMaterials[i]);
  }
}

void device::useModelList(ModelList *modelList) {
  guseState.currentModelList = modelList;
}

void device::renderPass() {
  const std::vector<int> &renderOrder =
      guseState.currentModelList->getRenderOrder();
  SoftCheck(renderOrder.size() > 0, LOG("[Warning] Empty render pass ! ", 0););
  for (int i = 0; i < renderOrder.size(); i++) {
    Model *model = guseState.currentModelList->models[renderOrder[i]];
    if (model->enabled) {
      model->draw();
    }
  }
}

void device::drawCall() {
  device::cullFrontFace(guseState.isFrontCulled());
  int vertexCount = guseState.currentMesh->vertexCount();
  SoftCheck(vertexCount > 0, {
    LOG("[Warning] vertexcount of mesh %p is 0", guseState.currentMesh);
  });

  if (guseState.currentMesh->ebo) {
    glDrawElements(guseState.currentModelConfiguration.renderMode,
                   guseState.currentMesh->vertexCount(), Standard::meshIndexGL,
                   (void *)0);
  } else {
    glDrawArrays(guseState.currentModelConfiguration.renderMode, 0,
                 guseState.currentMesh->vertexCount());
  }
}

DeviceParameters device::queryDeviceParameters() {
  DeviceParameters parameters;
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
                &parameters.maxTextureUnits);
  return parameters;
}

//---------------------[END DEVICE USE]

// --------------------[BEGIN FRAMEBUFFER]

GLuint FrameBuffer::createDepthStencilBuffer() {
  GLuint rbo = device::createRenderBuffer();
  device::bindRenderBuffer(rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, bufferWidth,
                        bufferHeight);
  return rbo;
}

void FrameBuffer::initialize() {
  _framebuffer = device::createFramebuffer();
  device::bindFrameBuffer(_framebuffer);
  colorAttachments.resize(attachmentsDefinition.size());
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
}

void FrameBuffer::begin(int screenWidth, int screenHeight) {
  resize(screenWidth, screenHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
  SoftCheck(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
            LOG("[DEVICE] Incomplete framebuffer %d ", _framebuffer););
  clearFramebuffer();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBuffer::end() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void FrameBuffer::addChannel(const FrameBufferAttachmentDescriptor &fbodef) {
  attachmentsDefinition.push(fbodef);
}

void FrameBuffer::setConfiguration(FrameBufferDescriptorFlags flags) {
  configuration = flags;
}

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
  if (depthMask)
    glDepthMask(GL_FALSE);
  device::useModelConfiguration(this);
  device::useMesh(mesh);
  device::useProgram(program);
  if (material != nullptr)
    device::useMaterial(material);

  device::useMaterial(node);
  device::drawCall();
  if (depthMask)
    glDepthMask(GL_TRUE);
}
//---------------------[MODEL END]
//---------------------[MODELLIST BEGIN]

ModelList::ModelList() { rootnode = shambhala::createNode(); }
const std::vector<int> &ModelList::getRenderOrder() {
  if (shouldSort) {
    modelindices.resize(models.size());
    for (int i = 0; i < modelindices.size(); i++)
      modelindices[i] = i;
    std::sort(modelindices.begin(), modelindices.end(),
              [&](int lhs, int rhs) { return *models[lhs] < *models[rhs]; });
  }
  return modelindices;
}

void ModelList::forceSorting() { shouldSort = true; }
//---------------------[MODELLIST END]

//---------------------[BEGIN COMPONENT METHODS]

int Mesh::vertexCount() {
  if (ebo == nullptr)
    return vbo->vertexBuffer.size() / vbo->vertexSize();
  return ebo->indexBuffer.size();
}
void Texture::addTextureResource(TextureResource *textureData) {
  this->textureData.push(textureData);
}

void ModelList::add(Model *model) {
  this->models.push(model);
  if (model->node == nullptr) {
    model->node = shambhala::createNode();
  }
  model->node->setParentNode(rootnode);
  forceSorting();
}

bool Texture::needsUpdate() {
  for (int i = 0; i < textureData.size(); i++) {
    if (textureData[i]->needsUpdate())
      return true;
  }
  return false;
}
//---------------------[END COMPONENT METHODS]

//---------------------[BEGIN RENDERCAMERA]

RenderCamera::RenderCamera() { hasCustomBindFunction = true; }
void RenderCamera::render(int frame, bool isRoot) {
  if (frame == currentFrame)
    return;

  currentFrame = frame;
  for (int i = 0; i < renderBindings.size(); i++) {
    renderBindings[i].renderCamera->render(frame);
  }

  engine.currentRenderCamera = this;
  device::useModelList(modelList);
  shambhala::setWorldMaterial(Standard::clas_worldMatRenderCamera, this);

  bool useFrameBuffer = frameBuffer != nullptr && !isRoot;

  if (useFrameBuffer) {
    frameBuffer->begin(viewport()->screenWidth, viewport()->screenHeight);
  }

  if (overrideProgram) {
    device::useProgram(overrideProgram);
    device::ignoreProgramBinding(true);
  }

  if (postprocessProgram) {
    static Mesh *screenmesh = util::createScreen();
    device::useMesh(screenmesh);
    device::useProgram(postprocessProgram);
  }

  if (postprocessProgram) {
    device::drawCall();
  } else {
    device::renderPass();
  }
  if (overrideProgram) {
    device::ignoreProgramBinding(false);
  }

  if (useFrameBuffer) {
    frameBuffer->end();
  }

  shambhala::setWorldMaterial(Standard::clas_worldMatRenderCamera,
                              engine.defaultMaterial);
}

// TODO: Fix actually call this function
void RenderCamera::bind(Program *activeProgram) {
  for (int i = 0; i < renderBindings.size(); i++) {
    if (renderBindings[i].attachmentIndex < 0) {
      UTexture texture;
      texture.mode = GL_TEXTURE_2D;
      texture.texID = renderBindings[i].renderCamera->frameBuffer->depthBuffer;
      texture.unit = Standard::tDepthTexture;
      device::useUniform(renderBindings[i].uniformAttribute, texture);
    } else {
      UTexture texture;
      texture.mode = GL_TEXTURE_2D;
      texture.texID = renderBindings[i]
                          .renderCamera->frameBuffer
                          ->colorAttachments[renderBindings[i].attachmentIndex];
      texture.unit = Standard::tAttachmentTexture + i;
      device::useUniform(renderBindings[i].uniformAttribute, texture);
    }
  }
}

void RenderCamera::addInput(RenderCamera *child, int attachmentIndex,
                            const char *uniformAttribute) {
  RenderBinding renderBinding;
  renderBinding.attachmentIndex = attachmentIndex;
  renderBinding.renderCamera = child;
  renderBinding.uniformAttribute = uniformAttribute;
  renderBindings.push(renderBinding);
}

void RenderCamera::addOutput(FrameBufferAttachmentDescriptor desc) {
  if (frameBuffer == nullptr)
    frameBuffer = shambhala::createFramebuffer();
  frameBuffer->addChannel(desc);
}

void RenderCamera::setFrameBufferConfiguration(
    FrameBufferDescriptorFlags flags) {
  if (frameBuffer == nullptr) {
    frameBuffer = shambhala::createFramebuffer();
  }
  frameBuffer->setConfiguration(flags);
}
//---------------------[END RENDERCAMERA]
//---------------------[BEGIN ENGINE]

void shambhala::loop_beginRenderContext() {
  guseState.clearState();
  gBindState.clearState();
  glViewport(0, 0, viewport()->screenWidth, viewport()->screenHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  device::bindVao(engine.vao);
}
void shambhala::loop_beginUIContext() {}
void shambhala::loop_endRenderContext() { viewport()->dispatchRenderEvents(); }
void shambhala::loop_endUIContext() {}
void shambhala::loop_step() {
  for (auto &ent : guseState.worldMaterials) {
    if (ent.second->needsFrameUpdate) {
      ent.second->update(viewport()->deltaTime);
    }
  }
}

// TODO FIX: glfw header incluse
bool shambhala::loop_shouldClose() { return viewport()->shouldClose(); }

void shambhala::destroyEngine() {}
//---------------------[BEGIN ENGINECONFIGURATION]
void shambhala::createEngine(EngineParameters parameters) {
  engine.controllers = parameters;
  engine.init();
}

void *shambhala::createWindow(const WindowConfiguration &configuration) {
  return viewport()->createWindow(configuration);
}
void shambhala::setActiveWindow(void *window) {
  viewport()->setActiveWindow(window);
  engine.prepareRender();
}

void shambhala::setWorldMaterial(int clas, Material *worldMaterial) {
  guseState.worldMaterials[clas] = worldMaterial;
}

void shambhala::addModel(Model *model) {
  engine.workingModelList->add(model);

  if (model->mesh == nullptr) {
    LOG("Warning, model %p does not have a mesh! ", model);
  }
}

void shambhala::setWorkingModelList(ModelList *modelList) {
  engine.workingModelList = modelList;
}
ModelList *shambhala::getWorkingModelList() { return engine.workingModelList; }

IViewport *shambhala::viewport() { return engine.controllers.viewport; }
IIO *shambhala::io() { return engine.controllers.io; }
ILogger *shambhala::log() { return engine.controllers.logger; }

//---------------------[END ENGINECONFIGURATION]

//---------------------[BEGIN ENGINECREATE]

ModelList *shambhala::createModelList() { return new ModelList; }
Texture *shambhala::createTexture() { return new Texture; }
Model *shambhala::createModel() { return new Model; }
Mesh *shambhala::createMesh() { return new Mesh; }
Program *shambhala::createProgram() { return new Program; }
FrameBuffer *shambhala::createFramebuffer() { return new FrameBuffer; }
Material *shambhala::createMaterial() { return new Material; }
Node *shambhala::createNode() { return new Node; }
RenderCamera *shambhala::createRenderCamera() {
  RenderCamera *result = new RenderCamera;
  result->modelList = engine.workingModelList;
  return result;
}
VertexBuffer *shambhala::createVertexBuffer() { return new VertexBuffer; }
IndexBuffer *shambhala::createIndexBuffer() { return new IndexBuffer; }

//---------------------[END ENGINECREATE]

//---------------------[BEGIN ENGINEUPDATE]

void shambhala::buildSortPass() { engine.workingModelList->forceSorting(); }
//---------------------[END ENGINEUPDATE]
//---------------------[END ENGINE]

//---------------------[BEGIN HELPER]

#include <unordered_map>

using Key = unsigned long;
template <typename T, typename Container> struct LoaderMap {
  std::unordered_map<Key, T *> cache;
  std::unordered_map<T *, int> countMap;

  template <typename... Args> T *get(Args &&...args) {
    Key key = Container::computeKey(std::forward<Args>(args)...);
    auto it = cache.find(key);
    if (it != cache.end()) {
      int counter = countMap[it->second];
      if (counter >= 1) {
        countMap[it->second]++;
        return it->second;
      }
    }
    T *result = cache[key] = Container::create(std::forward<Args>(args)...);
    countMap[result] = 1;
    return result;
  }

  void unload(T *ptr) {
    int count = --countMap[ptr];
    if (count < 1) {
      delete ptr;
    }
  }
};

struct ProgramContainer : public LoaderMap<Program, ProgramContainer> {
  static Program *create(const char *fs, const char *vs) {
    Program *program = shambhala::createProgram();
    program->shaders[FRAGMENT_SHADER].file = resource::ioMemoryFile(fs);
    program->shaders[VERTEX_SHADER].file = resource::ioMemoryFile(vs);
    return program;
  }

  static Key computeKey(const char *fs, const char *vs) {
    return Key(fs) * 5 + Key(vs) * 3;
  }
};

static ProgramContainer programContainer;

Program *loader::loadProgram(const char *fs, const char *vs) {
  return programContainer.get(fs, vs);
}
void loader::unloadProgram(Program *program) {
  programContainer.unload(program);
}
