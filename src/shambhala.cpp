#include "shambhala.hpp"
#include "adapters/log.hpp"
#include "adapters/video.hpp"
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

using namespace shambhala;

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
  VideoDeviceParameters gpu_params;

  const char *errorProgramFS = "programs/error.fs";
  const char *errorProgramVS = "programs/error.vs";

  Program *defaultProgram = nullptr;
  Material *defaultMaterial = nullptr;

  bool prepared = false;
  void init() {

    gpu_params = vid()->queryDeviceParameters();

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
bool Uniform::bind(GLuint program, GLuint glUniformID) const {

  // TODO: This switch should be deleted...
  switch (type) {
  case UniformType::VEC2:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_VEC2,
                       (void *)&VEC2[0]);
    break;
  case UniformType::VEC3:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_VEC3,
                       (void *)&VEC3[0]);
    break;
  case UniformType::VEC2PTR:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_VEC3,
                       (void *)VEC2PTR, count);
    break;
  case UniformType::FLOATPTR:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_FLOAT,
                       (void *)FLOATPTR, count);
    break;
  case UniformType::VEC3PTR:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_VEC3,
                       (void *)VEC3PTR, count);
    break;
  case UniformType::VEC4:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_VEC4,
                       (void *)&VEC4[0]);
    break;
  case UniformType::MAT2:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_MAT2,
                       (void *)&MAT2[0][0]);
    break;
  case UniformType::MAT3:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_MAT3,
                       (void *)&MAT3[0][0]);
    break;
  case UniformType::MAT4:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_MAT4,
                       (void *)&MAT4[0][0]);
    break;
  case UniformType::FLOAT:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_FLOAT,
                       (void *)&FLOAT);
    break;
  case UniformType::BOOL:
  case UniformType::INT:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_INT,
                       (void *)&INT);
    break;
  case UniformType::INTPTR:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_INT, INTPTR);
    break;
  case UniformType::UTEXTURE:
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_SAMPLER,
                       (void *)&UTEXTURE.textureID);
    break;
  case UniformType::ITEXTURE:
    GLuint textureID = ITEXTURE->gl();
    vid()->bindUniform(program, glUniformID, video::SH_UNIFORM_SAMPLER,
                       &textureID);
    break;
  }

  return true;
}
int VertexBuffer::vertexSize() const {
  if (_vertexsize != -1)
    return _vertexsize;
  int vs = 0;
  for (int i = 0; i < attributes.size(); i++) {
    vs += attributes[i].size;
  }
  return _vertexsize = vs * sizeof(float);
}

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

  DrawCallArgs getDrawArgs() {
    DrawCallArgs args;
    args.vertexCount = currentMesh->vertexCount();
    args.indexed = currentMesh->ebo != nullptr;
    args.instanceCount = 0;
    return args;
  }
};
static UseState guseState;

bool Shader::use(GLint type) {

  IResource *file = this->file.file();
  if (gl_shader == -1 || file) {
    if (gl_shader != -1) {
      vid()->disposeShader(gl_shader);
    }

    video::ShaderDesc desc;
    desc.type = type;
    desc.data = (const char *)file->read()->data;
    desc.name = file->resourcename.c_str();
    gl_shader = vid()->compileShader(desc);

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
      vid()->disposeProgram(gl_shaderProgram);
    }

    video::ProgramDesc desc;
    desc.shaderCount = SHADER_TYPE_COUNT;
    desc.shaders = shaders;
    desc.status = &status;
    gl_shaderProgram = vid()->compileProgram(desc);
    errored = !status;
  }

  return gl_shaderProgram;
}
void Program::use() {

  if (errored)
    return engine.defaultProgram->use();

  if (guseState.currentProgram == this || guseState.ignoreProgramUse)
    return;

  vid()->bindProgram(gl());
  guseState.currentProgram = this;

  for (int i = 0; i < engine.materialsStack.size(); i++) {
    bind(engine.materialsStack[i]);
  }
}

void IndexBuffer::use() {

  if (this == guseState.currentIndexBuffer)
    return;

  guseState.currentIndexBuffer = this;

  if (indexBuffer.size() == 0)
    return;

  if (gl_ebo == -1) {
    gl_ebo = vid()->createBuffer({GL_ELEMENT_ARRAY_BUFFER});
  }

  if (needsUpdate()) {

    vid()->uploadBuffer(descUpload(indexBuffer.span(), gl_ebo));
    signalAck();
  }

  vid()->bindBuffer(gl_ebo);
}

void VertexBuffer::use() {

  // Skip if buffer is already bound and updated
  if (this == guseState.currentVertexBuffer && !needsUpdate())
    return;

  guseState.currentVertexBuffer = this;

  SoftCheck(vertexBuffer.size() > 0, { LOG("Vertex buffer empty!\n", 0); });

  if (vertexBuffer.size() == 0)
    return;

  if (gl_vbo == -1) {
    gl_vbo = vid()->createBuffer({GL_ARRAY_BUFFER});
  }

  // Create vertexBuffer
  if (needsUpdate()) {
    vid()->uploadBuffer(descUpload(vertexBuffer.span(), gl_vbo));
    signalAck();
  }

  vid()->bindBuffer(gl_vbo);

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

    video::AttributeDesc attr;
    attr.buffer = gl_vbo;
    attr.index = index;
    attr.size = size;
    attr.stride = stride;
    attr.offset = offset * sizeof(float);
    attr.divisor = divisor;
    vid()->bindAttribute(attr);
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

  video::TextureDesc desc;
  desc.cubemap = textureMode == GL_TEXTURE_CUBE_MAP;
  desc.clamp = clamp;
  desc.useNeareast = useNeareast;

  if (gl_textureID == -1) {
    gl_textureID = vid()->createTexture(desc);
  }

  if (needsUpdate()) {
    GLenum target = textureMode;

    for (int i = 0; i < textureData.size(); i++) {
      TextureResource *textureResource = textureData[i].file();
      if (textureResource) {
        GLenum target = textureMode == GL_TEXTURE_CUBE_MAP
                            ? (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i)
                            : GL_TEXTURE_2D;

        video::TextureUploadDesc desc;
        desc.buffer = textureResource->textureBuffer;
        desc.width = textureResource->width;
        desc.height = textureResource->height;
        desc.textureID = gl_textureID;
        desc.target = target;
        desc.format = descTextureFormat(textureResource->hdrSpace,
                                        textureResource->components);
        vid()->uploadTexture(desc);

        textureData[i].signalAck();
      }
    }
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
  GLuint programId = gl();
  GLuint uniformId = vid()->getUniform(programId, name);

  if (uniformId != -1)
    value.bind(programId, uniformId);
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

void shambhala::renderPass() {
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
  return vid()->createRenderbuffer(
      {GL_DEPTH24_STENCIL8, bufferWidth, bufferHeight});
}

void FrameBuffer::initialize() {

  video::TextureUploadDesc text;
  text.width = bufferWidth;
  text.height = bufferHeight;
  text.textureID = 0;
  text.target = GL_TEXTURE_2D;

  video::TextureDesc textureDesc;

  if (gl_framebuffer == -1) {

    colorAttachments.resize(attachmentsDefinition.size());
    for (int i = 0; i < attachmentsDefinition.size(); i++) {
      colorAttachments[i] = vid()->createTexture(textureDesc);
      text.textureID = colorAttachments[i];
      text.format = attachmentsDefinition[i];
      vid()->uploadTexture(text);
    }

    video::FrameBufferDesc fbodef;
    if (combinedDepthStencil()) {
      if (configuration & USE_RENDER_BUFFER) {
        gl_stencilDepthBuffer = createDepthStencilBuffer();
        fbodef.renderBufferAttachment = gl_stencilDepthBuffer;
      } else {
        gl_stencilDepthBuffer = vid()->createTexture({});
        text.textureID = gl_stencilDepthBuffer;
        vid()->uploadTexture(text);
        fbodef.depthStencilAttachment = gl_stencilDepthBuffer;
      }
    }

    else {
      if (configuration & USE_DEPTH) {
        gl_depthBuffer = vid()->createTexture(descDepthTexture());
        vid()->uploadTexture(
            descDepthUpload(bufferWidth, bufferHeight, gl_depthBuffer));
        fbodef.depthAttachment = gl_depthBuffer;
      }

      if (configuration & USE_STENCIL) {
        gl_stencilBuffer = vid()->createTexture(descStencilTexture());
        vid()->uploadTexture(
            descStencilUpload(bufferWidth, bufferHeight, gl_stencilBuffer));
        fbodef.stencilAttachment = gl_stencilBuffer;
      }
    }

    fbodef.attachmentCount = colorAttachments.size();
    fbodef.attachments = &colorAttachments[0];
    gl_framebuffer = vid()->createFramebuffer(fbodef);
  } else {
    for (int i = 0; i < attachmentsDefinition.size(); i++) {
      text.textureID = colorAttachments[i];
      text.format = attachmentsDefinition[i];
      vid()->uploadTexture(text);
    }
  }
}

void FrameBuffer::dispose() {}

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
  vid()->bindFrameBuffer(gl_framebuffer);
  vid()->set(video::SH_CLEAR_COLOR, clearColor);
  vid()->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shambhala::viewport()->fakeViewportSize(screenWidth, screenHeight);
  shambhala::updateViewport();
}

void FrameBuffer::end() {
  vid()->bindFrameBuffer(0);
  shambhala::viewport()->restoreViewport();
  shambhala::updateViewport();
}

void FrameBuffer::addOutput(video::TextureFormat format) {
  attachmentsDefinition.push(format);
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

  this->use();
  mesh->use();
  program->use();
  program->bind(node);

  if (hint_modelid == engine.hint_selected_modelid &&
      hint_selection_material != nullptr) {

    program->bind(hint_selection_material);
  } else if (material != nullptr)
    program->bind(material);

  DrawCallArgs args = guseState.getDrawArgs();
  args.instanceCount = instanceCount;
  vid()->drawCall(args);
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

void RenderCamera::render() { shambhala::renderPass(); }

GLuint RenderCameraOutput::gl() {

  if (camera->currentFrame != engine.currentFrame) {
    camera->currentFrame = engine.currentFrame;

    engine_clearState();
    camera->begin();
    camera->render();
    camera->end();
    engine_clearState();
  }

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

void shambhala::engine_clearState() { guseState.clearState(); }

void shambhala::engine_prepareRender() {

  vid()->set(GL_CULL_FACE, true);
  vid()->set(GL_DEPTH_TEST, true);
  vid()->set(GL_MULTISAMPLE, true);
  vid()->set(GL_BLEND, true);

  vid()->set(GL_SRC_ALPHA, GL_SRC_ALPHA);
  vid()->set(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);

  if (engine.vao == -1) {
    engine.vao = vid()->createVAO();

#ifdef DEBUG
    vid()->enableDebug(true);
#endif
  }
  vid()->bindVao(engine.vao);
}

void shambhala::engine_prepareDeclarativeRender() {

  vid()->set(video::SH_CLEAR_COLOR, engine.renderConfig->clearColor);

  viewport()->fakeViewportSize(engine.renderConfig->virtualWidth,
                               engine.renderConfig->virtualHeight);

  updateViewport();
  vid()->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void shambhala::updateViewport() {
  vid()->setViewport(viewport()->getScreenWidth(),
                     viewport()->getScreenHeight());
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
void shambhala::createEngine(EngineControllers controllers) {
  engine.controllers = controllers;
  engine.controllers.audio->initDevice();
  engine.controllers.video->initDevice();
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
video::IVideo *shambhala::vid() { return engine.controllers.video; }

void shambhala::drawCall() { vid()->drawCall(guseState.getDrawArgs()); }

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
