#include "shambhala.hpp"
#include "adapters/log.hpp"
#include "core/core.hpp"
#include "core/resource.hpp"
#include "ext/math.hpp"
#include "simple_vector.hpp"
#include "standard.hpp"
#include <algorithm>
#include <cstdio>
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

struct SelectionHint {
  int hint_selected_modelid = -1;
};

struct Engine : public SelectionHint {
  EngineControllers controllers;

  // Global State
  simple_vector<ModelList *> workingLists;
  simple_vector<Material *> materialsStack;
  simple_vector<LogicComponent *> components;

  // Root configuration
  Material *wCamera;
  RenderConfiguration *renderConfig;
  Node *rootNode;
  GLuint vao = -1;
  int currentFrame = 0;

  // Misc
  DeviceParameters gpu_params;

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

Node::Node() { clean = false; }

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
  activeProgram->bind(Standard::uTransformMatrix, Uniform(getCombinedMatrix()));
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
  case UniformType::INTPTR:
    glUniform1iv(glUniformID, count, INTPTR);
    break;
  case UniformType::ITEXTURE:
    glUniform1i(glUniformID,
                device::bindTexture(ITEXTURE->gl(), ITEXTURE->getMode()));
    break;
  case UniformType::UTEXTURE:
    glUniform1i(glUniformID,
                device::bindTexture(UTEXTURE.textureID, UTEXTURE.textureMode));
    break;
  }

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
                         GLuint *vbo, GLenum mode) {
  if (*vbo == -1) {
    glGenBuffers(1, vbo);
  }
  device::bindVbo(*vbo);
  glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size(), vertexBuffer.data(), mode);
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

  GLuint nextUnboundUnit = 0;

  void clearState() {
    currentVao = -1;
    currentProgram = -1;
    nextUnboundUnit = 0;
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

int device::bindTexture(GLuint textureId, GLenum mode) {

  device::bindTexture(textureId, mode, gBindState.nextUnboundUnit);
  return gBindState.nextUnboundUnit++;
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
    gBindState.boundAttributes[index] = vbo;
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride,
                          (void *)(size_t(offset)));

    glEnableVertexAttribArray(index);

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
  // TODO: We should cache this...
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

bool Shader::use(GLint type) {

  IResource *file = this->file.file();
  if (gl_shader == -1 || file) {
    if (gl_shader != -1) {
      device::disposeShader(gl_shader);
    }

    gl_shader = device::compileShader((const char *)file->read()->data, type,
                                      file->resourcename);
    this->file.signalAck();
    return true;
  }
  return false;
}

GLuint Program::gl() {

  // Check program compilation with shaders
  bool programUpdate = gl_shaderProgram == -1;

  /// Compile shaders
  for (int i = 0; i < SHADER_TYPE_COUNT; i++) {
    if (shaders[i] != nullptr) {
      programUpdate |= shaders[i]->use(i + GL_FRAGMENT_SHADER);
    }
  }

  // Link program
  if (programUpdate) {

    compilationCount++;
    GLuint shaders[SHADER_TYPE_COUNT + 1] = {0};
    int currentShader = 0;

    for (int i = 0; i < SHADER_TYPE_COUNT; i++) {
      if (this->shaders[i] != nullptr) {
        shaders[currentShader++] = this->shaders[i]->gl_shader;
      }
    }
    GLint status;

    if (gl_shaderProgram != -1) {
      device::disposeProgram(gl_shaderProgram);
    }

    gl_shaderProgram = device::compileProgram(shaders, &status);
    errored = !status;
  }

  return gl_shaderProgram;
}
void Program::use() {

  if (errored)
    return engine.defaultProgram->use();

  if (guseState.currentProgram == this || guseState.ignoreProgramUse)
    return;

  device::bindProgram(gl());
  guseState.currentProgram = this;

  for (int i = 0; i < engine.materialsStack.size(); i++) {
    bind(engine.materialsStack[i]);
  }
}

void device::ignoreProgramBinding(bool ignore) {
  guseState.ignoreProgramUse = ignore;
}

void IndexBuffer::use() {

  if (this == guseState.currentIndexBuffer)
    return;

  guseState.currentIndexBuffer = this;
  if (indexBuffer.size() && (needsUpdate() || gl_ebo == -1)) {

    device::createEBO(indexBuffer, &gl_ebo);
    signalAck();
  }

  device::bindEbo(gl_ebo);
}

void VertexBuffer::use() {

  // Skip if buffer is already bound and updated
  if (this == guseState.currentVertexBuffer && !needsUpdate())
    return;

  guseState.currentVertexBuffer = this;

  SoftCheck(vertexBuffer.size() > 0, { LOG("Vertex buffer empty!\n", 0); });

  if (vertexBuffer.size() == 0)
    return;

  // Create vertexBuffer
  if (gl_vbo == -1 || (vertexBuffer.size() && needsUpdate())) {

    device::createVBO(vertexBuffer, &gl_vbo, mode);
    vboSize = vertexBuffer.size();

    // Reallocate buffer memory
  } else if (needsUpdate() && vertexBuffer.size() <= vboSize) {
    device::bindVbo(gl_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBuffer.size(),
                    vertexBuffer.data());
  }

  signalAck();
  device::bindVbo(gl_vbo);

  // Bind attributes
  SoftCheck(attributes.size() != 0, {
    LOG("[Warning] Vertexbuffer with not attributes! %d",
        (int)attributes.size());
  });
  int offset = 0;
  int stride = vertexSize();
  for (int i = 0; i < attributes.size(); i++) {
    int index = attributes[i].index;
    int divisor = attributes[i].attributeDivisor;
    int size = attributes[i].size;

    device::bindAttribute(gl_vbo, index, size, stride, offset * sizeof(float),
                          divisor);
    offset += attributes[i].size;
  }
}

void Mesh::use() {

  if (guseState.currentMesh == this)
    return;

  vbo->use();
  if (ebo)
    ebo->use();

  guseState.currentMesh = this;
  guseState.meshCullFrontFace = invertedFaces;
}

GLuint Texture::gl() {

  if (gl_textureID == -1)
    gl_textureID = textureMode == GL_TEXTURE_CUBE_MAP
                       ? device::createCubemap()
                       : device::createTexture(!useNeareast, clamp);

  if (needsUpdate()) {
    GLenum target = textureMode;
    device::bindTexture(gl_textureID, target);
    for (int i = 0; i < textureData.size(); i++) {
      TextureResource *textureResource = textureData[i].file();
      if (textureResource) {
        GLenum target = textureMode == GL_TEXTURE_CUBE_MAP
                            ? (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i)
                            : GL_TEXTURE_2D;
        device::uploadTexture(target, textureResource->textureBuffer,
                              textureResource->width, textureResource->height,
                              textureResource->components,
                              textureResource->hdrSpace);
        textureData[i].signalAck();
      }
    }
    glGenerateMipmap(target);
  }
  return gl_textureID;
}

// TODO:Improve quality, also check if caching is any useful
void ModelConfiguration::use() {

  ModelConfiguration *configuration = this;
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

void Program::bind(const char *name, Uniform value) {
  use();
  GLuint uniformId = device::getUniform(gl(), name);

  if (uniformId != -1)
    value.bind(uniformId);
}

void Program::bind(Material *mat) {

  mat->bind(this);
  for (auto &uniform : mat->uniforms) {
    bind(uniform.first.c_str(), uniform.second);
  }

  for (int i = 0; i < mat->childMaterials.size(); i++) {
    bind(mat->childMaterials[i]);
  }
}

void device::renderPass() {
  const std::vector<int> &renderOrder =
      guseState.currentModelList->getRenderOrder();
  for (int i = 0; i < renderOrder.size(); i++) {
    Model *model = guseState.currentModelList->models[renderOrder[i]];
    if (model->isEnabled()) {
      model->draw();
    }
  }

  for (int i = 0; i < componentCount(); i++) {
    getComponent(i)->render();
  }
}

void device::drawCall(DrawCallArgs args) {
  // device::cullFrontFace(guseState.isFrontCulled());
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

GLuint FrameBufferOutput::gl() {
  return framebuffer->getOutputAttachment(attachmentIndex);
}
FrameBufferOutput *FrameBuffer::getOutputTexture(int index) {
  auto *out = new FrameBufferOutput;
  out->framebuffer = this;
  out->attachmentIndex = index;
  return out;
}
GLuint FrameBuffer::getOutputAttachment(int index) {
  if (index == Standard::attachmentDepthBuffer)
    return gl_depthBuffer;
  return colorAttachments[index];
}

GLuint FrameBuffer::createDepthStencilBuffer() {
  GLuint rbo = device::createRenderBuffer();
  device::bindRenderBuffer(rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, bufferWidth,
                        bufferHeight);
  return rbo;
}

void FrameBuffer::initialize() {
  if (gl_framebuffer == -1)
    gl_framebuffer = device::createFramebuffer();
  device::bindFrameBuffer(gl_framebuffer);

  if (colorAttachments.size() != attachmentsDefinition.size()) {

    int lastTexture = colorAttachments.size();
    colorAttachments.resize(attachmentsDefinition.size());
    glGenTextures(attachmentsDefinition.size() - lastTexture,
                  &colorAttachments[lastTexture]);
  }

  for (int i = 0; i < colorAttachments.size(); i++) {
    device::bindTexture(colorAttachments[i], GL_TEXTURE_2D, 0);
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
      gl_stencilDepthBuffer = createDepthStencilBuffer();
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                GL_RENDERBUFFER, gl_stencilDepthBuffer);
    } else {
      gl_stencilDepthBuffer = device::createTexture(false);
      device::uploadDepthStencilTexture(gl_stencilDepthBuffer, bufferWidth,
                                        bufferHeight);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                             GL_TEXTURE_2D, gl_stencilDepthBuffer, 0);
    }
  } else {
    if (configuration & USE_DEPTH) {
      gl_depthBuffer = device::createTexture(false);
      device::uploadDepthTexture(gl_depthBuffer, bufferWidth, bufferHeight);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                             gl_depthBuffer, 0);
    }

    if (configuration & USE_STENCIL) {
      gl_stencilBuffer = device::createTexture(false);
      device::uploadStencilTexture(gl_stencilBuffer, bufferWidth, bufferHeight);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                             GL_TEXTURE_2D, gl_stencilBuffer, 0);
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

  if (combinedDepthStencil()) {
    if (configuration & USE_RENDER_BUFFER)
      glDeleteRenderbuffers(1, &gl_stencilDepthBuffer);
    else
      glDeleteTextures(1, &gl_stencilDepthBuffer);
  } else {
    if (configuration & USE_DEPTH) {
      glDeleteTextures(1, &gl_depthBuffer);
    }
    if (configuration & USE_STENCIL) {
      glDeleteTextures(1, &gl_stencilBuffer);
    }
  }
}

void FrameBuffer::resize(int screenWidth, int screenHeight) {
  if (bufferWidth == screenWidth && bufferHeight == screenHeight)
    return;
  if (gl_framebuffer != -1)
    dispose();

  bufferWidth = screenWidth;
  bufferHeight = screenHeight;

  initialize();
}

void FrameBuffer::begin() { begin(desiredWidth, desiredHeight); }

void FrameBuffer::begin(int screenWidth, int screenHeight) {

  if (screenWidth < 0)
    screenWidth = viewport()->getScreenWidth() / -desiredWidth;
  if (screenHeight < 0)
    screenHeight = viewport()->getScreenHeight() / -desiredHeight;

  resize(screenWidth, screenHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, gl_framebuffer);
  SoftCheck(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
            LOG("[DEVICE] Incomplete framebuffer %d ", gl_framebuffer););
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shambhala::viewport()->fakeViewportSize(screenWidth, screenHeight);
  shambhala::updateViewport();
}

void FrameBuffer::end() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  shambhala::viewport()->restoreViewport();
  shambhala::updateViewport();
}

void FrameBuffer::addOutput(const FrameBufferAttachmentDescriptor &fbodef) {
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
  this->use();
  mesh->use();
  program->use();
  program->bind(node);

  if (hint_modelid == engine.hint_selected_modelid &&
      hint_selection_material != nullptr) {

    program->bind(hint_selection_material);
  } else if (material != nullptr)
    program->bind(material);

  device::drawCall(*this);
  if (depthMask)
    glDepthMask(GL_TRUE);
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

void ModelList::use() { guseState.currentModelList = this; }

//---------------------[MODELLIST END]

//---------------------[LOGIC COMPONENT BEGIN]

void LogicComponent::setName(const char *name) {
  EngineComponent<LogicComponent>::setName(name);
}
void LogicComponent::add(Model *model) { shambhala::addModel(model); }

//---------------------[MATERIAL BEGIN]

void Material::addMaterial(Material *mat) { childMaterials.push(mat); }
bool Material::has(const std::string &uniformName) {
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

void Material::deserialize(io_buffer buffer) {}

void Material::setSetupProgram(Program *program) {

  static auto iscapital = [](char x) {
    if (x >= 'A' && x <= 'Z')
      return 1;

    else
      return 0;
  };

  GLuint shaderProgram = program->gl();
  setupProgramCompilationCount = program->getCompilationCount();
  setupProgram = program;
  int uniformCount;
  glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &uniformCount);
  static int buffer_size = 128;
  static char buffer[128];
  for (int i = 0; i < uniformCount; i++) {
    GLsizei length;
    GLint size;
    GLenum type;
    glGetActiveUniform(shaderProgram, (GLuint)i, buffer_size, &length, &size,
                       &type, buffer);
    std::string name(buffer);

    // IGNORE WORLD MATERIALS TODO:
    if (name[0] == 'u' && iscapital(name[1]))
      continue;

    if (has(name))
      continue;

    switch (type) {
    case GL_FLOAT:
      set(name, 0.0f);
      break;
    case GL_INT:
      set(name, 0);
      break;
    case GL_FLOAT_VEC2:
      set(name, glm::vec2(0.0f));
      break;
    case GL_FLOAT_VEC3:
      set(name, glm::vec3(0.0f));
      break;
    }
  }
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

void ModelList::remove(Model *model) {
  this->models.removeNShiftObject(model);
  forceSorting();
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

PostProcessCamera::PostProcessCamera(const char *path)
    : PostProcessCamera(
          util::createScreenProgram(resource::ioMemoryFile(path))) {}

PostProcessCamera::PostProcessCamera(Program *program) {
  this->postProcessProgram = program;
}

void PostProcessCamera::render() {
  util::renderScreen(this, postProcessProgram);
}

void RenderCamera::render() { shambhala::device::renderPass(); }

GLuint RenderCameraOutput::gl() {

  Program *lastBoundProgram = guseState.currentProgram;
  if (camera->currentFrame != engine.currentFrame) {
    camera->currentFrame = engine.currentFrame;

    engine_clearState();
    camera->begin();
    camera->render();
    camera->end();
    engine_clearState();
  }

  lastBoundProgram->use();
  return camera->getOutputTexture(attachmentIndex)->gl();
}

RenderCameraOutput *RenderCamera::renderOutput(int attachmentIndex) {

  RenderCameraOutput *result = new RenderCameraOutput;
  result->camera = this;
  result->attachmentIndex = attachmentIndex;
  return result;
}

//---------------------[END RENDERCAMERA]

//---------------------[BEGIN ENGINE]

void shambhala::loop_begin() {}
void shambhala::loop_end() {}

void shambhala::loop_beginRenderContext(int frame) {
  engine_clearState();
  engine_prepareRender();
  engine_prepareDeclarativeRender();
  viewport()->notifyFrame(frame);
  engine.currentFrame = frame;
}

void shambhala::engine_clearState() {
  guseState.clearState();
  gBindState.clearState();
}

void shambhala::engine_prepareRender() {

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
  info.mouseRay = ext::createRay((worldmats::Camera *)engine.wCamera,
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
  engine.controllers.audio->initDevice();
  engine.init();
}

void *shambhala::createWindow(const WindowConfiguration &configuration) {
  return viewport()->createWindow(configuration);
}
void shambhala::setActiveWindow(void *window) {
  viewport()->setActiveWindow(window);
  viewport()->imguiInit(4, 2);
}

void shambhala::addModel(Model *model) { getWorkingModelList()->add(model); }
void shambhala::removeModel(Model *model) {
  getWorkingModelList()->remove(model);
}

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
audio::IAudio *shambhala::aud() { return engine.controllers.audio; }

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

void shambhala::destroyModel(Model *model) { delete model; }

const WorldMatCollection &shambhala::getWorldMaterials() {
  return engine.materialsStack;
}

void shambhala::pushMaterial(Material *mat) {
  engine.materialsStack.push(mat);
  if (mat->hint_isCamera)
    engine.wCamera = mat;
}
void shambhala::popMaterial() { engine.materialsStack.pop(); }

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
