#include "shambhala.hpp"
#include "adapters/log.hpp"
#include "adapters/video.hpp"
#include "core/core.hpp"
#include "core/resource.hpp"
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
  std::vector<ModelList *> workingLists;
  std::vector<Material *> materialsStack;
  std::vector<LogicComponent *> components;

  // Root configuration
  Material *wCamera;
  RenderConfiguration *renderConfig;
  Node *rootNode;
  int currentFrame = 0;

  // Misc
  video::VideoDeviceParameters gpu_params;

  const char *errorProgramFS = "programs/error.fs";
  const char *errorProgramVS = "programs/error.vs";

  Program *defaultProgram = nullptr;
  Material *defaultMaterial = nullptr;

  void init() {

    gpu_params = video::queryDeviceParameters();

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

bool Uniform::bind(GLuint program, GLuint glUniformID) const {

  // TODO: This switch should be deleted...
  switch (type) {
  case UniformType::VEC2:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_VEC2,
                       (void *)&VEC2[0]);
    break;
  case UniformType::VEC3:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_VEC3,
                       (void *)&VEC3[0]);
    break;
  case UniformType::VEC2PTR:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_VEC3,
                       (void *)VEC2PTR, count);
    break;
  case UniformType::FLOATPTR:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_FLOAT,
                       (void *)FLOATPTR, count);
    break;
  case UniformType::VEC3PTR:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_VEC3,
                       (void *)VEC3PTR, count);
    break;
  case UniformType::VEC4:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_VEC4,
                       (void *)&VEC4[0]);
    break;
  case UniformType::MAT2:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_MAT2,
                       (void *)&MAT2[0][0]);
    break;
  case UniformType::MAT3:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_MAT3,
                       (void *)&MAT3[0][0]);
    break;
  case UniformType::MAT4:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_MAT4,
                       (void *)&MAT4[0][0]);
    break;
  case UniformType::FLOAT:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_FLOAT,
                       (void *)&FLOAT);
    break;
  case UniformType::BOOL:
  case UniformType::INT:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_INT,
                       (void *)&INT);
    break;
  case UniformType::INTPTR:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_INT, INTPTR,
                       count);
    break;
  case UniformType::UTEXTURE:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_SAMPLER,
                       (void *)&UTEXTURE.textureID);
    break;
  case UniformType::ITEXTURE:
    GLuint textureID = ITEXTURE->gl();
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_SAMPLER,
                       &textureID);
    break;
  }

  return true;
}
//--------------------------[END COMPILE]

//--------------------------[BEGIN CREATE]

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

struct UseState {

  // Current use state
  Program *currentProgram = nullptr;
  Mesh *currentMesh = nullptr;
  ModelList *currentModelList = nullptr;
  VertexBuffer *currentVertexBuffer = nullptr;
  IndexBuffer *currentIndexBuffer = nullptr;

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
  }

  bool isFrontCulled() { return meshCullFrontFace ^ modelCullFrontFace; }

  video::DrawCallArgs getDrawArgs() {
    video::DrawCallArgs args;
    args.vertexCount = currentMesh->vertexCount();
    args.indexed = currentMesh->ebo != nullptr;
    args.instanceCount = 0;
    args.frontCulled = isFrontCulled();
    return args;
  }
};
static UseState guseState;

bool Shader::use(GLint type) {

  IResource *file = this->file.file();
  if (gl_shader == -1 || file) {
    if (gl_shader != -1) {
      video::disposeShader(gl_shader);
    }

    video::ShaderDesc desc;
    desc.type = type;
    desc.data = (const char *)file->read()->data;
    desc.name = file->resourcename.c_str();
    gl_shader = video::compileShader(desc);

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

    if (gl_shaderProgram != -1) {
      video::disposeProgram(gl_shaderProgram);
    }

    video::ProgramDesc desc;
    desc.shaderCount = SHADER_TYPE_COUNT;
    desc.shaders = shaders;
    gl_shaderProgram = video::compileProgram(desc);
    errored = video::statusProgramCompilation().errored;
  }

  return gl_shaderProgram;
}
void Program::use() {

  video::bindProgram(gl());

  if (errored)
    return engine.defaultProgram->use();

  if (guseState.currentProgram == this || guseState.ignoreProgramUse)
    return;

  guseState.currentProgram = this;

  for (int i = 0; i < engine.materialsStack.size(); i++) {
    bind(engine.materialsStack[i]);
  }
}

void Program::bind(const char *name, Uniform value) {
  gl();
  GLuint uniformId = video::getUniform(gl_shaderProgram, name);

  if (uniformId != -1)
    value.bind(gl_shaderProgram, uniformId);
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

GLuint Texture::gl() {

  if (gl_textureID == -1) {
    gl_textureID = video::createTexture((video::TextureDesc) * this);
  }

  if (needsUpdate()) {
    for (int i = 0; i < textureData.size(); i++) {
      TextureResource *textureResource = textureData[i].file();
      if (textureResource) {
        GLenum target = video::TextureDesc::mode == GL_TEXTURE_CUBE_MAP
                            ? (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i)
                            : GL_TEXTURE_2D;

        video::TextureUploadDesc desc;
        desc.buffer = textureResource->textureBuffer;
        desc.width = textureResource->width;
        desc.height = textureResource->height;
        desc.textureID = gl_textureID;
        desc.target = target;
        desc.format = video::descTextureFormat(textureResource->hdrSpace,
                                               textureResource->components);
        video::uploadTexture(desc);

        textureData[i].signalAck();
      }
    }
  }
  return gl_textureID;
}

//---------------------[END DEVICE USE]

// --------------------[BEGIN FRAMEBUFFER]

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
  video::drawCall(args);
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
  this->models.push_back(model);
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

void shambhala::renderPass() {
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

  Program *lastBoundProgram = guseState.currentProgram;
  if (camera->currentFrame != engine.currentFrame) {
    camera->currentFrame = engine.currentFrame;

    engine_clearState();
    camera->begin();
    camera->render();
    camera->end();
    engine_clearState();
  }

  if (lastBoundProgram != nullptr)
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

void shambhala::engine_clearState() { guseState.clearState(); }

void shambhala::engine_prepareRender() {

  // Hummmm TODO: refactor this
  static bool initialized = false;
  if (!initialized) {
    video::initDevice();
    initialized = true;
#ifdef DEBUG
    video::enableDebug(true);
#endif
  }

  // video::set(GL_CULL_FACE, true);
  // video::set(GL_DEPTH_TEST, true);
  // video::set(GL_MULTISAMPLE, true);
  video::set(GL_BLEND, true);

  video::set(GL_SRC_ALPHA, GL_SRC_ALPHA);
  video::set(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  video::stepBegin();
}

void shambhala::engine_prepareDeclarativeRender() {

  video::set(video::SH_CLEAR_COLOR,
             glm::vec4(engine.renderConfig->clearColor, 1.0));

  updateViewport();
  video::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void shambhala::updateViewport() {
  video::setViewport(viewport()->getScreenWidth(),
                     viewport()->getScreenHeight());
}
void shambhala::endViewport() { viewport()->restoreViewport(); }
void shambhala::loop_beginUIContext() { viewport()->imguiBeginRender(); }

void shambhala::loop_endRenderContext() {
  viewport()->restoreViewport();
  viewport()->notifyFrameEnd();
  viewport()->dispatchRenderEvents();

  video::stepEnd();
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

  StepInfo info{};
  for (int i = 0; i < engine.components.size(); i++) {
    engine.components[i]->step(info);
  }
}
void shambhala::destroyEngine() {}

//---------------------[BEGIN ENGINECONFIGURATION]
void shambhala::createEngine(EngineControllers controllers) {
  engine.controllers = controllers;
  engine.init();
}

void *shambhala::createWindow(const WindowConfiguration &configuration) {
  return viewport()->createWindow(configuration);
}
void shambhala::setActiveWindow(void *window) {
  viewport()->setActiveWindow(window);
  viewport()->imguiInit(3, 3);
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

video::DrawCallArgs shambhala::getDefaultArgs() {
  return guseState.getDrawArgs();
}
void shambhala::drawCall() { video::drawCall(getDefaultArgs()); }

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
    static int i = 0;
    return i++;
    // return loader::Key((void *)resource);
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
