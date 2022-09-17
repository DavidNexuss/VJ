#include "shambhala.hpp"
#include "simple_vector.hpp"
#include "standard.hpp"
#include <algorithm>
#include <ext/util.hpp>
#include <stbimage/stb_image.h>
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
    device::useTexture(SAMPLER2D);
    glUniform1i(glUniformID, SAMPLER2D.unit);
    break;
  case UniformType::DYNAMIC_TEXTURE:
    device::useTexture(DYNAMIC_TEXTURE);
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

GLuint device::createVAO(
    const simple_vector<MeshLayout::MeshLayoutAttribute> &attributes) {
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
  GLuint currentProgram = -1;
  GLuint currentVao = -1;
  GLuint currentVbo = -1;
  GLuint currentEbo = -1;
  int activeTextureUnit = -1;

  bool cullFrontFace = false;
  void clearState() {}
};
static BindState gBindState;

void device::bindVao(GLuint vao) {
  glBindVertexArray(vao);
  gBindState.currentVao = vao;
}
void device::bindVbo(GLuint vbo) {
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  gBindState.currentVbo = vbo;
}
void device::bindEbo(GLuint ebo) {
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
  if (gBindState.boundTextures[textureUnit] != textureId) {
    gBindState.boundTextures[textureUnit] = textureId;
    if (gBindState.activeTextureUnit != textureUnit) {
      glActiveTexture(textureUnit);
      gBindState.activeTextureUnit = textureUnit;
    }
    glBindTexture(mode, textureId);
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
  MeshLayout *currentMeshLayout = nullptr;
  ModelConfiguration currentModelConfiguration;
  bool hasModelConfiguration = false;

  // Culling information
  bool modelCullFrontFace = false;
  bool meshCullFrontFace = false;

  std::unordered_map<int, Material *> worldMaterials;
  void bindUniforms(Material *material) {}
  void clearState() {
    currentProgram = nullptr;
    currentMesh = nullptr;
    currentMeshLayout = nullptr;
    hasModelConfiguration = false;
  }

  bool isFrontCulled() { return meshCullFrontFace ^ modelCullFrontFace; }
};
static UseState guseState;

void device::useProgram(Program *program) {
  if (guseState.currentProgram == program)
    return;

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
    program->errored = status;
  }

  device::bindProgram(program->shaderProgram);
  guseState.currentProgram = program;

  // Bind worldMaterials
  for (auto &worldmat : guseState.worldMaterials) {
    worldmat.second->bind(program);
  }
}
void device::useMeshLayout(MeshLayout *layout) {
  if (layout == guseState.currentMeshLayout)
    return;

  if (layout->vao == -1)
    layout->vao = device::createVAO(layout->attributes);
  device::bindVao(layout->vao);
  guseState.currentMeshLayout = layout;
}
void device::useMesh(Mesh *mesh) {
  if (guseState.currentMesh == mesh)
    return;
  device::useMeshLayout(mesh->meshLayout);
  if (mesh->indexBuffer.size() && (mesh->needsEBOUpdate || mesh->ebo == -1)) {
    device::createEBO(mesh->indexBuffer, &mesh->ebo);
    mesh->needsEBOUpdate = false;
  }
  if (mesh->vertexBuffer.size() && (mesh->needsVBOUpdate || mesh->vbo == -1)) {
    device::createVBO(mesh->vertexBuffer, &mesh->vbo);

    int offset = 0;
    for (int i = 0; i < mesh->meshLayout->attributes.size(); i++) {
      glVertexAttribPointer(mesh->meshLayout->attributes[i].index,
                            mesh->meshLayout->attributes[i].size, GL_FLOAT,
                            GL_FALSE, mesh->meshLayout->vertexSize(),
                            (void *)(offset * sizeof(float)));
      glEnableVertexAttribArray(mesh->meshLayout->attributes[i].index);
      offset += mesh->meshLayout->attributes[i].size;
    }
    mesh->needsVBOUpdate = false;
  }
  device::bindVbo(mesh->vbo);
  device::bindEbo(mesh->ebo);
  guseState.currentMesh = mesh;
  guseState.meshCullFrontFace = mesh->invertedFaces;
}

void device::useTexture(UTexture texture) {
  device::bindTexture(texture.texID, texture.mode, texture.unit);
}

void device::useTexture(Texture *texture) {

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
  value.bind(device::getUniform(guseState.currentProgram->shaderProgram, name));
}

void device::useMaterial(Material *material) {
  for (auto &uniform : material->uniforms) {
    useUniform(uniform.first.c_str(), uniform.second);
  }
  if (material->hasCustomBindFunction) {
    material->bind(guseState.currentProgram);
  }
}

void device::drawCall() {
  device::cullFrontFace(guseState.isFrontCulled());
  if (guseState.currentMesh->ebo != -1)
    glDrawElements(guseState.currentModelConfiguration.renderMode,
                   guseState.currentMesh->vertexCount(), Standard::meshIndexGL,
                   (void *)0);
  else
    glDrawArrays(guseState.currentModelConfiguration.renderMode, 0,
                 guseState.currentMesh->vertexCount());
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
  return vertexBuffer.size() / meshLayout->vertexSize();
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
//---------------------[BEGIN ENGINE]

struct Engine {
  EngineControllers controllers;
  ModelList *workingModelList;

  void init() { workingModelList = shambhala::createModelList(); }

  void prepareRender() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
  }

  void cleanup() {}
};

static Engine engine;

void shambhala::loop_beginRenderContext() {
  guseState.clearState();
  gBindState.clearState();
  glViewport(0, 0, viewport()->screenWidth, viewport()->screenHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void shambhala::loop_beginUIContext() {}
void shambhala::loop_endRenderContext() { viewport()->dispatchRenderEvents(); }
void shambhala::loop_declarativeRender() { renderPass(); }
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

IViewport *shambhala::viewport() { return engine.controllers.viewport; }
IIO *shambhala::io() { return engine.controllers.io; }
ILogger *shambhala::log() { return engine.controllers.logger; }

//---------------------[END ENGINECONFIGURATION]

//---------------------[BEGIN ENGINECREATE]

void shambhala::rendertarget_prepareRender() { engine.prepareRender(); }
ModelList *shambhala::createModelList() { return new ModelList; }
Texture *shambhala::createTexture() { return new Texture; }
Model *shambhala::createModel() { return new Model; }
Mesh *shambhala::createMesh() { return new Mesh; }
MeshLayout *shambhala::createMeshLayout() { return new MeshLayout; }
Program *shambhala::createProgram() { return new Program; }
FrameBuffer *shambhala::createFramebuffer() { return new FrameBuffer; }
Material *shambhala::createMaterial() { return new Material; }
Node *shambhala::createNode() { return new Node; }

//---------------------[END ENGINECREATE]

//---------------------[BEGIN ENGINEUPDATE]

void shambhala::buildSortPass() { engine.workingModelList->forceSorting(); }
void shambhala::renderPass() {
  const std::vector<int> &renderOrder =
      engine.workingModelList->getRenderOrder();
  for (int i = 0; i < renderOrder.size(); i++) {
    Model *model = engine.workingModelList->models[renderOrder[i]];
    if (model->enabled) {
      model->draw();
    }
  }
}
//---------------------[END ENGINEUPDATE]
//---------------------[END ENGINE]
