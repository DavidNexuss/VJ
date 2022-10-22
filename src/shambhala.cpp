#include "shambhala.hpp"
#include "adapters/log.hpp"
#include "core/core.hpp"
#include "core/resource.hpp"
#include "ext/math.hpp"
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

#define TEXTURE_CACHING 1

using namespace shambhala;

void glError(GLenum source, GLenum type, GLuint id, GLenum severity,
             GLsizei length, const GLchar *message, const void *userParam) {

  if (severity >= 37190 &&
      (id == GL_INVALID_OPERATION || type == GL_INVALID_OPERATION)) {

    std::cerr << "[GL " << severity << "] " << message << std::endl;
    throw std::runtime_error{"error"};
  }
}

inline void clearFramebuffer() { glClearColor(0.0, 0.0, 0.0, 0.0); }
inline void clearDefault() { glClearColor(0.0, 0.0, 0.0, 1.0); }

struct SelectionHint {
  int hint_selected_modelid = -1;
};

struct Engine : public SelectionHint {
  EngineControllers controllers;
  simple_vector<ModelList *> workingLists;
  DeviceParameters gpu_params;
  RenderConfiguration *renderConfig;
  Node *rootNode;
  simple_vector<LogicComponent *> components;
  GLuint vao = -1;
  int currentFrame;

  const char *errorProgramFS = "programs/error.fs";
  const char *errorProgramVS = "programs/error.vs";

  Program *defaultProgram = nullptr;
  Material *defaultMaterial = nullptr;

  bool prepared = false;
  void init() {
    gpu_params = device::queryDeviceParameters();

    defaultProgram = loader::loadProgram(errorProgramFS, errorProgramVS);
    defaultMaterial = shambhala::createMaterial();
    renderConfig = new RenderConfiguration;
    rootNode = new Node;
    rootNode->setName("root");
  }

  void cleanup() { loader::unloadProgram(defaultProgram); }
};

static Engine engine;
//---------------------[BEGIN NODE]

Node::Node() {
  clean = false;
  hasCustomBindFunction = true;
}

void Node::setDirty() {
  clean = false;
  enableclean = false;
  for (Node *child : children) {
    child->setDirty();
  }
}
void Node::addChildNode(Node *childNode) {
  children.push_back(childNode);
  childNode->setDirty();
}
void Node::setParentNode(Node *parentNode) {
  if (this->parentNode) {
    this->parentNode->children.remove(this);
  }
  this->parentNode = parentNode;
  parentNode->addChildNode(this);
}

void Node::setOffset(glm::vec3 offset) {
  transformMatrix[3] = glm::vec4(offset, 1.0);
  setDirty();
}
void Node::setTransformMatrix(const glm::mat4 &newVal) {
  transformMatrix = newVal;
  setDirty();
}

void Node::transform(const glm::mat4 &newval) {
  glm::mat4 oldMatrix = getTransformMatrix();
  setTransformMatrix(newval * oldMatrix);
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

bool Node::isEnabled() {
  if (enableclean)
    return cachedenabled;

  enableclean = true;
  if (!enabled)
    return cachedenabled = false;
  if (parentNode == nullptr)
    return cachedenabled = true;
  return cachedenabled = parentNode->isEnabled();
}

void Node::setEnabled(bool pEnable) {
  enabled = pEnable;
  setDirty();
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
  case UniformType::VEC2PTR:
    glUniform2fv(glUniformID, count, &VEC2PTR->x);
    break;
  case UniformType::FLOATPTR:
    glUniform1fv(glUniformID, count, FLOATPTR);
    break;
  case UniformType::VEC3PTR:
    glUniform3fv(glUniformID, count, &VEC3PTR->x);
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
//--------------------------[BEGIN MESH]
int VertexBuffer::vertexSize() const {
  if (_vertexsize != -1)
    return _vertexsize;
  int vs = 0;
  for (int i = 0; i < attributes.size(); i++) {
    vs += attributes[i].size;
  }
  return _vertexsize = vs * sizeof(float);
}
//--------------------------[END MESH]

//--------------------------[BEGIN UPLOAD]
void device::uploadTexture(GLenum target, unsigned char *texturebuffer,
                           int width, int height, int components, bool hdr) {

  const static int internalFormat[] = {GL_RED, GL_RG8, GL_RGB8, GL_RGBA8};
  const static int externalFormat[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
  glTexImage2D(target, 0, internalFormat[components - 1], width, height, 0,
               externalFormat[components - 1],
               hdr ? GL_FLOAT : GL_UNSIGNED_BYTE, texturebuffer);
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
                             const std::string &resourcename) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &data, NULL);
  glCompileShader(shader);
  GLint compileStatus = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus == GL_FALSE) {

    GLint errorSize;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorSize);
    char *errorLog = new char[errorSize];
    glGetShaderInfoLog(shader, errorSize, &errorSize, errorLog);
    glDeleteShader(shader);

    LOG("[DEVICE] Error compiling shader %d: %s, code: \n%s\n error: \n%s",
        shader, resourcename.c_str(), data, errorLog);
    delete[] errorLog;

  } else {
    LOG("[DEVICE] Shader compiled successfully %d: %s", shader,
        resourcename.c_str());
  }
  return shader;
}

GLuint device::compileProgram(GLuint *shaders, GLint *status) {
  if (*shaders == 0) {
    LOG("[DEVICE] Null program !! ", 0);
    return -1;
  }
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

    glDeleteProgram(program);
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

GLuint device::createEBO(const simple_vector<Standard::meshIndex> &indexBuffer,
                         GLuint *ebo) {
  if (*ebo == -1) {
    glGenBuffers(1, ebo);
  }
  device::bindEbo(*ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               indexBuffer.size() * sizeof(Standard::meshIndex),
               indexBuffer.data(), GL_STATIC_DRAW);
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
GLuint device::createTexture(bool filter, bool clamp) {
  GLuint textureId;
  glGenTextures(1, &textureId);
  device::bindTexture(textureId, GL_TEXTURE_2D);
  if (clamp) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  if (filter) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST_MIPMAP_LINEAR);
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

  GLuint currentVao = -1;

  void clearState() {
    currentVao = -1;
    currentProgram = -1;
  }
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

  WorldMatCollection worldMaterials;
  float lineWidth = 0.0f;

  void clearState() {
    currentProgram = nullptr;
    currentMesh = nullptr;
    currentVertexBuffer = nullptr;
    currentIndexBuffer = nullptr;
    hasModelConfiguration = false;
  }

  bool isFrontCulled() { return meshCullFrontFace ^ modelCullFrontFace; }
};
static UseState guseState;

bool device::useShader(Shader *shader, GLint type) {
  IResource *file = shader->file.file();
  if (shader->shader == -1 || file) {
    if (shader->shader != -1) {
      device::disposeShader(shader->shader);
    }

    shader->shader = device::compileShader((const char *)file->read()->data,
                                           type, file->resourcename);
    shader->file.signalAck();
    return true;
  }
  return false;
}
void device::useProgram(Program *program) {

  // SoftCheck(program != nullptr, LOG("[Warning] Trying to use a null
  // program!", 0););

  if (program == nullptr || program->errored) {
    device::useProgram(engine.defaultProgram);
    return;
  }

  if (guseState.currentProgram == program || guseState.ignoreProgramUse)
    return;
  // Check program compilation with shaders
  bool programUpdate = program->shaderProgram == -1;

  /// Compile shaders
  for (int i = 0; i < SHADER_TYPE_COUNT; i++) {
    if (program->shaders[i] != nullptr) {
      programUpdate |=
          device::useShader(program->shaders[i], i + GL_FRAGMENT_SHADER);
    }
  }

  // Link program
  if (programUpdate) {

    program->compilationCount++;
    GLuint shaders[SHADER_TYPE_COUNT + 1] = {0};
    int currentShader = 0;

    for (int i = 0; i < SHADER_TYPE_COUNT; i++) {
      if (program->shaders[i] != nullptr) {
        shaders[currentShader++] = program->shaders[i]->shader;
      }
    }
    GLint status;

    if (program->shaderProgram != -1) {
      device::disposeProgram(program->shaderProgram);
    }

    program->shaderProgram = device::compileProgram(shaders, &status);
    program->errored = !status;
  }

  device::bindProgram(program->shaderProgram);
  guseState.currentProgram = program;
  useWorldMaterials();
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

  // Skip if buffer is already bound and updated
  if (vertexbuffer == guseState.currentVertexBuffer &&
      !vertexbuffer->updateData)
    return;

  guseState.currentVertexBuffer = vertexbuffer;

  SoftCheck(vertexbuffer->vertexBuffer.size() > 0,
            { LOG("Vertex buffer empty!\n", 0); });

  if (vertexbuffer->vertexBuffer.size() == 0)
    return;

  // Create vertexBuffer
  if (vertexbuffer->vbo == -1 ||
      vertexbuffer->vboSize < vertexbuffer->vertexBuffer.size()) {

    device::createVBO(vertexbuffer->vertexBuffer, &vertexbuffer->vbo);
    vertexbuffer->vboSize = vertexbuffer->vertexBuffer.size();

    // Reallocate buffer memory
  } else if (vertexbuffer->updateData &&
             vertexbuffer->vertexBuffer.size() <= vertexbuffer->vboSize) {
    device::bindVbo(vertexbuffer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexbuffer->vertexBuffer.size(),
                    vertexbuffer->vertexBuffer.data());
  }

  vertexbuffer->updateData = false;
  device::bindVbo(vertexbuffer->vbo);

  // Bind attributes
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
    texture->_textureID =
        texture->textureMode == GL_TEXTURE_CUBE_MAP
            ? device::createCubemap()
            : device::createTexture(!texture->useNeareast, texture->clamp);

  if (texture->needsUpdate()) {
    GLenum target = texture->textureMode;
    device::bindTexture(texture->_textureID, target);
    for (int i = 0; i < texture->textureData.size(); i++) {
      TextureResource *textureResource = texture->textureData[i].file();
      if (textureResource) {
        GLenum target = texture->textureMode == GL_TEXTURE_CUBE_MAP
                            ? (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i)
                            : GL_TEXTURE_2D;
        device::uploadTexture(target, textureResource->textureBuffer,
                              textureResource->width, textureResource->height,
                              textureResource->components,
                              textureResource->hdrSpace);
        texture->textureData[i].signalAck();
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
  if (guseState.lineWidth != configuration->lineWidth) {
    glLineWidth(configuration->lineWidth);
    guseState.lineWidth = configuration->lineWidth;
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

void device::useWorldMaterials() {

  // Bind worldMaterials
  for (auto &worldmat : guseState.worldMaterials) {
    useMaterial(worldmat.second);
  }
}
void device::useMaterial(Material *material) {
  if (material == nullptr)
    return;

  SoftCheck(material != nullptr, log()->log("[Warning] Null material use"););

  if (material->setupProgram) {
    shambhala::setupMaterial(material, material->setupProgram);
  }

  if (material->hasCustomBindFunction) {
    material->bind(guseState.currentProgram);
  }
  for (auto &uniform : material->uniforms) {
    useUniform(uniform.first.c_str(), uniform.second);
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
    if (model->isEnabled()) {
      model->draw();
    }
  }
}

void device::drawCall(DrawCallArgs args) {
  device::cullFrontFace(guseState.isFrontCulled());
  int vertexCount = guseState.currentMesh->vertexCount();
  SoftCheck(vertexCount > 0, {
    LOG("[Warning] vertexcount of mesh %p is 0", guseState.currentMesh);
  });

  if (guseState.currentMesh->ebo) {
    if (args.instance_count != 0) {
      glDrawElementsInstanced(guseState.currentModelConfiguration.renderMode,
                              guseState.currentMesh->vertexCount(),
                              Standard::meshIndexGL, (void *)0,
                              args.instance_count);
    } else
      glDrawElements(guseState.currentModelConfiguration.renderMode,
                     guseState.currentMesh->vertexCount(),
                     Standard::meshIndexGL, (void *)0);
  } else {

    if (args.instance_count != 0) {
      glDrawArraysInstanced(guseState.currentModelConfiguration.renderMode, 0,
                            guseState.currentMesh->vertexCount(),
                            args.instance_count);
    } else
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

    if (attachmentsDefinition[i].useNeareast) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

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
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBuffer::end() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void FrameBuffer::addChannel(const FrameBufferAttachmentDescriptor &fbodef) {
  attachmentsDefinition.push(fbodef);
}

void FrameBuffer::setConfiguration(FrameBufferDescriptorFlags flags) {
  configuration = flags;
}

int FrameBuffer::getWidth() { return bufferWidth; }
int FrameBuffer::getHeight() { return bufferHeight; }

//---------------------[FRAMEBUFFER END]
//---------------------[MODEL BEGIN]
bool Model::operator<(const Model &model) const {

  return zIndex < model.zIndex ||
         (zIndex == model.zIndex &&
          (program < model.program ||
           (program == model.program &&
            (mesh < model.mesh ||
             (mesh == model.mesh && material == model.material)))));
}
bool Model::ready() const { return program != nullptr && mesh != nullptr; }

bool Model::isEnabled() {
  if (node == nullptr)
    node = shambhala::createNode();

  if (!node->isEnabled())
    return false;
  return true;
}
void Model::draw() {
  if (node == nullptr)
    node = shambhala::createNode();

  if (depthMask)
    glDepthMask(GL_FALSE);
  device::useModelConfiguration(this);
  device::useMesh(mesh);
  device::useProgram(program);

  if (hint_modelid == engine.hint_selected_modelid &&
      hint_selection_material != nullptr) {

    device::useMaterial(hint_selection_material);
  } else if (material != nullptr)
    device::useMaterial(material);

  device::useMaterial(node);
  device::drawCall(*this);
  if (depthMask)
    glDepthMask(GL_TRUE);
}

Model *Model::createInstance() {
  Model *newInstance = shambhala::createModel();
  *newInstance = *this;
  if (newInstance->node != nullptr) {
    newInstance->node = shambhala::createNode(newInstance->node);
  }
  return newInstance;
}

Node *Model::getNode() {
  if (node == nullptr)
    return node = shambhala::createNode();
  return node;
}
//---------------------[MODEL END]
//---------------------[MODELLIST BEGIN]

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

int ModelList::size() const { return models.size(); }
Model *ModelList::get(int index) const { return models[index]; }
ModelList *ModelList::createInstance() {
  ModelList *newInstance = shambhala::createModelList();
  *newInstance = *this;
  for (int i = 0; i < models.size(); i++) {
    models[i] = models[i]->createInstance();
  }
  newInstance->forceSorting();
  return newInstance;
}
//---------------------[MODELLIST END]

//---------------------[LOGIC COMPONENT BEGIN]

void LogicComponent::setName(const char *name) {
  EngineComponent<LogicComponent>::setName(name);
}
void LogicComponent::add(Model *model) { shambhala::addModel(model); }
//---------------------[LOGIC COMPONENT END]

//---------------------[BEGIN COMPONENT METHODS]

//---------------------[BEGIN ENGINE RESOURCE]

void EngineResource::save() {
  if (configurationResource == nullptr || dirty == false)
    return;

  io_buffer data = serialize();
  *configurationResource->read() = data;
  configurationResource->write();
  dirty = false;
}

void EngineResource::load() {
  if (configurationResource == nullptr)
    return;
  deserialize(*configurationResource->read());
}

void EngineResource::setConfigurationResource(IResource *resource) {
  configurationResource = resource;
}

io_buffer EngineResource::serialized() {
  if (serialized_buffer.data != nullptr)
    delete[] serialized_buffer.data;

  dirty = false;
  return serialized_buffer = serialize();
}

const char *EngineResource::configurationResourcePath() {
  if (configurationResource == nullptr)
    return nullptr;
  return configurationResource->resourcename.c_str();
}

//---------------------[END ENGINE RESOURCE]
//---------------------[MATERIAL BEGIN]

void Material::addMaterial(Material *mat) { childMaterials.push(mat); }
bool Material::isDefined(const std::string &uniformName) {
  return uniforms.count(uniformName);
}
void Material::popNextMaterial() {
  childMaterials.resize(childMaterials.size() - 1);
}

io_buffer Material::serialize() {
  for (auto &it : uniforms) {
    switch (it.second.type) {
    case FLOAT:
      serializer()->serialize(it.first.c_str(), it.second.FLOAT, 0);
      break;
    case VEC2:
      serializer()->serialize(it.first.c_str(), it.second.VEC2, 0);
      break;
    }
  }

  return serializer()->end();
}

void Material::deserialize(io_buffer buffer) {
  /*
  serializer()->deserialize(buffer);
  int n = serializer()->deserializeEntryCount();
  for (int i = 0; i < n; i++) {
    ISerializer::DeserializeEntry entry = serializer()->deserializeEntryIt(i);
    switch (entry.type) {}
  } */
}

//---------------------[MATERIAL END]
int Mesh::vertexCount() {
  if (ebo == nullptr)
    return vbo->vertexBuffer.size() / vbo->vertexSize();
  return ebo->indexBuffer.size();
}
void Texture::addTextureResource(TextureResource *textureData) {
  ResourceHandlerAbstract<TextureResource> handler;
  handler.acquire(textureData);
  this->textureData.push(handler);
}

VertexAttribute Mesh::getAttribute(int attribIndex) {
  for (int i = 0; i < vbo->attributes.size(); i++) {
    if (vbo->attributes[i].index == attribIndex) {
      VertexAttribute attrib = vbo->attributes[i];
      attrib.sourceData = (float *)&vbo->vertexBuffer[0];
      attrib.stride = vbo->vertexSize();
      return attrib;
    }
  }
  return VertexAttribute{};
}

void ModelList::add(Model *model) {
  this->models.push(model);
  forceSorting();

  if (model->mesh == nullptr) {
    LOG("Warning, model %p does not have a mesh! ", model);
  }
}

bool Texture::needsUpdate() {
  for (int i = 0; i < textureData.size(); i++) {
    if (textureData[i].file())
      return true;
  }
  return false;
}
//---------------------[END COMPONENT METHODS]

//---------------------[BEGIN RENDERCAMERA]

RenderCamera::RenderCamera() { hasCustomBindFunction = true; }

RenderCamera *RenderCamera::render(const RenderShot &shot) {
  beginRender(shot);
  endRender(shot);
  return this;
}

void RenderCamera::beginRender(const RenderShot &shot) {

  if (shot.currentFrame == currentFrame)
    return;

  for (int i = 0; i < renderBindings.size(); i++) {
    renderBindings[i].renderCamera->render(shot);
  }

  shambhala::setWorldMaterial(Standard::clas_worldMatRenderCamera, this);

  if (width != 0 || height != 0) {
    shambhala::viewport()->fakeViewportSize(width, height);
    shambhala::updateViewport();
  }
}

void RenderCamera::endRender(const RenderShot &shot) {

  if (shot.currentFrame == currentFrame)
    return;

  currentFrame = shot.currentFrame;
  engine.currentFrame = currentFrame;

  bool useFrameBuffer = frameBuffer != nullptr && !shot.isRoot;

  if (useFrameBuffer) {
    frameBuffer->begin(viewport()->getScreenWidth(),
                       viewport()->getScreenHeight());
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
    device::useModelList(shot.scenes[modelListInput]);
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

  if (width != 0 || height != 0) {
    viewport()->restoreViewport();
    shambhala::updateViewport();
  }
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

void RenderCamera::setModelList(int modelListIndex) {
  this->modelListInput = modelListIndex;
}

void RenderCamera::addInput(RenderCamera *child, int attachmentIndex,
                            const char *uniformAttribute) {
  RenderBinding renderBinding;
  renderBinding.attachmentIndex = attachmentIndex;
  renderBinding.renderCamera = child;
  renderBinding.uniformAttribute = uniformAttribute;
  renderBindings.push(renderBinding);
}

void RenderCamera::addDummyInput(RenderCamera *child) {
  dummyInput.push(child);
}
RenderCamera *RenderCamera::getDummyInput(int index) {
  return dummyInput[index];
}
int RenderCamera::getDummyInputCount() { return dummyInput.size(); }

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

int RenderCamera::getWidth() {
  if (frameBuffer != nullptr)
    return frameBuffer->getWidth();
  return viewport()->getScreenWidth();
}
int RenderCamera::getHeight() {
  if (frameBuffer != nullptr)
    return frameBuffer->getHeight();
  return viewport()->getScreenHeight();
}

void RenderCamera::setSize(int width, int height) {
  this->width = width;
  this->height = height;
}

//---------------------[END RENDERCAMERA]

//---------------------[BEGIN RENDERSHOT]
void RenderShot::updateFrame() { this->currentFrame++; }
//---------------------[END RENDERSHOT]
//---------------------[BEGIN ENGINE]

void shambhala::loop_begin() {}
void shambhala::loop_end() {}

void shambhala::loop_beginRenderContext(int frame) {
  engine_clearState();
  engine_prepareRender();
  engine_prepareDeclarativeRender();
  viewport()->notifyFrame(frame);
}

void shambhala::engine_clearState() {
  guseState.clearState();
  gBindState.clearState();
}

void shambhala::engine_prepareRender() {

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
  glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if (engine.vao == -1) {
    engine.vao = device::createVAO();

#ifdef DEBUG
    glDebugMessageCallback(glError, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
#endif
  }
  device::bindVao(engine.vao);
}

void shambhala::engine_prepareDeclarativeRender() {

  glClearColor(engine.renderConfig->clearColor.x,
               engine.renderConfig->clearColor.y,
               engine.renderConfig->clearColor.z, 1.0);

  viewport()->fakeViewportSize(engine.renderConfig->virtualWidth,
                               engine.renderConfig->virtualHeight);

  updateViewport();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void shambhala::updateViewport() {
  glViewport(0, 0, viewport()->getScreenWidth(), viewport()->getScreenHeight());
}
void shambhala::endViewport() { viewport()->restoreViewport(); }
void shambhala::loop_beginUIContext() { viewport()->imguiBeginRender(); }

void shambhala::loop_endRenderContext() {
  viewport()->restoreViewport();
  viewport()->notifyFrameEnd();
  viewport()->dispatchRenderEvents();
}

void shambhala::loop_endUIContext() { viewport()->imguiEndRender(); }

bool shambhala::loop_shouldClose() { return viewport()->shouldClose(); }

StepInfo shambhala::getStepInfo() {

  StepInfo info;
  // TODO: Hard cast
  info.mouseRay =
      ext::createRay((worldmats::Camera *)getWorldMaterial(Standard::wCamera),
                     viewport()->getMouseViewportCoords());
  return info;
}
void shambhala::loop_io_sync_step() { io()->filewatchMonitor(); }
void shambhala::loop_componentUpdate() {

  StepInfo info;
  for (int i = 0; i < engine.components.size(); i++) {
    engine.components[i]->step(info);
  }
}
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
  viewport()->imguiInit(4, 2);
}

Material *shambhala::getWorldMaterial(WorldMatID clas) {
  return guseState.worldMaterials[clas];
}
void shambhala::setWorldMaterial(WorldMatID clas, Material *worldMaterial) {
  guseState.worldMaterials[clas] = worldMaterial;
}

void shambhala::addModel(Model *model) { getWorkingModelList()->add(model); }

void shambhala::setWorkingModelList(ModelList *modelList) {
  if (modelList == nullptr) {
    engine.workingLists.pop();
  } else {
    engine.workingLists.push(modelList);
  }
}

Node *shambhala::getRootNode() { return engine.rootNode; }
ModelList *shambhala::getWorkingModelList() {
  return engine.workingLists[engine.workingLists.size() - 1];
}

ISerializer *shambhala::serializer() { return engine.controllers.serializer; }
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
Shader *shambhala::createShader() { return new Shader; }
FrameBuffer *shambhala::createFramebuffer() { return new FrameBuffer; }
Material *shambhala::createMaterial() { return new Material; }

Node *shambhala::createNode(const char *name, Node *old) {
  Node *newInstance = new Node;
  newInstance->setName(name);
  if (old != nullptr) {
    newInstance->transformMatrix = old->transformMatrix;
    newInstance->children.clear();
    for (Node *node : old->children) {
      newInstance->children.push_back(shambhala::createNode(node));
    }
  } else {
    newInstance->setParentNode(engine.rootNode);
  }
  return newInstance;
}

const WorldMatCollection &shambhala::getWorldMaterials() {
  return guseState.worldMaterials;
}

Node *shambhala::createNode() {
  return shambhala::createNode(nullptr, nullptr);
}
Node *shambhala::createNode(const char *componentName) {
  return shambhala::createNode(componentName, nullptr);
}
Node *shambhala::createNode(Node *old) {
  return shambhala::createNode(nullptr, old);
}
void shambhala::addComponent(LogicComponent *component) {
  engine.components.push(component);
}

LogicComponent *shambhala::getComponent(int index) {
  return engine.components[index];
}
int shambhala::componentCount() { return engine.components.size(); }

RenderCamera *shambhala::createRenderCamera() { return new RenderCamera; }
VertexBuffer *shambhala::createVertexBuffer() { return new VertexBuffer; }
IndexBuffer *shambhala::createIndexBuffer() { return new IndexBuffer; }

void shambhala::disposeModelList(ModelList *list) {}

void shambhala::hint_selectionpass() {
  engine.hint_selected_modelid = util::doSelectionPass(getWorkingModelList());
}

bool shambhala::input_mouse_free() { return engine.hint_selected_modelid <= 0; }

//---------------------[END ENGINECREATE]

//---------------------[BEGIN ENGINEUPDATE]

void shambhala::buildSortPass() { getWorkingModelList()->forceSorting(); }
//---------------------[END ENGINEUPDATE]
//---------------------[END ENGINE]

//---------------------[BEGIN HELPER]

loader::Key loader::computeKey(const char *str) {

  unsigned long hash = 5381;
  int c;

  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

struct ShaderContainer : public loader::LoaderMap<Shader, ShaderContainer> {

  static Shader *create(IResource *resource) {
    Shader *shader = shambhala::createShader();
    shader->file.acquire(resource);
    return shader;
  }

  static loader::Key computeKey(IResource *resource) {
    return loader::Key(resource);
  }
};
struct ProgramContainer : public loader::LoaderMap<Program, ProgramContainer> {
  static Program *create(const char *fs, const char *vs) {
    Program *program = shambhala::createProgram();
    program->shaders[FRAGMENT_SHADER] = loader::loadShader(fs);
    program->shaders[VERTEX_SHADER] = loader::loadShader(vs);
    return program;
  }

  static loader::Key computeKey(const char *fs, const char *vs) {
    return loader::computeKey(fs) * 5 + loader::computeKey(vs) * 3;
  }
};

struct TextureContainer : public loader::LoaderMap<Texture, TextureContainer> {
  static Texture *create(const char *path, int channelCount) {
    Texture *texture = shambhala::createTexture();
    TextureResource *resource = resource::stbiTextureFile(path, channelCount);
    texture->addTextureResource(resource);
    return texture;
  }

  static loader::Key computeKey(const char *path, int channelCount) {
    return loader::computeKey(path) + channelCount;
  }
};

static ProgramContainer programContainer;
static ShaderContainer shaderContainer;
static TextureContainer textureContainer;

Program *loader::loadProgram(const char *fs, const char *vs) {
  return programContainer.get(fs, vs);
}
Shader *loader::loadShader(const char *path) {
  return loader::loadShader(resource::ioMemoryFile(path));
}

Shader *loader::loadShader(IResource *resource) {
  return shaderContainer.get(resource);
}

void loader::unloadProgram(Program *program) {
  programContainer.unload(program);
}

Program *loader::getProgram(int index) {
  return programContainer.linearCache[index];
}

int loader::programCount() { return programContainer.linearCache.size(); }

Shader *loader::getShader(int index) {
  return shaderContainer.linearCache[index];
}
int loader::shaderCount() { return shaderContainer.linearCache.size(); }

Texture *loader::loadTexture(const char *path, int channelCount) {
  return textureContainer.get(path, channelCount);
}

static int iscapital(char x) {
  if (x >= 'A' && x <= 'Z')
    return 1;

  else
    return 0;
}
void shambhala::setupMaterial(Material *material, Program *program) {
  device::useProgram(program);
  material->setupProgramCompilationCount = program->compilationCount;
  material->setupProgram = program;
  int uniformCount;
  glGetProgramiv(program->shaderProgram, GL_ACTIVE_UNIFORMS, &uniformCount);
  static int buffer_size = 128;
  static char buffer[128];
  for (int i = 0; i < uniformCount; i++) {
    GLsizei length;
    GLint size;
    GLenum type;
    glGetActiveUniform(program->shaderProgram, (GLuint)i, buffer_size, &length,
                       &size, &type, buffer);
    std::string name(buffer);

    // IGNORE WORLD MATERIALS TODO:
    if (name[0] == 'u' && iscapital(name[1]))
      continue;

    if (material->isDefined(name))
      continue;

    switch (type) {
    case GL_FLOAT:
      material->set(name, 0.0f);
      break;
    case GL_INT:
      material->set(name, 0);
      break;
    case GL_FLOAT_VEC2:
      material->set(name, glm::vec2(0.0f));
      break;
    case GL_FLOAT_VEC3:
      material->set(name, glm::vec3(0.0f));
      break;
    }
  }
}
