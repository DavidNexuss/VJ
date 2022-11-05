#pragma once
#include "adapters/audio.hpp"
#include "adapters/io.hpp"
#include "adapters/log.hpp"
#include "adapters/serialize.hpp"
#include "adapters/video.hpp"
#include "adapters/viewport.hpp"
#include "core/component.hpp"
#include "core/core.hpp"
#include "core/resource.hpp"
#include "simple_vector.hpp"
#include <glm/glm.hpp>
#include <list>
#include <memory>
#include <standard.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace shambhala {

struct Material;
using WorldMatID = int;
using WorldMatCollection = simple_vector<Material *>;

enum ShaderType {
  FRAGMENT_SHADER = 0,
  VERTEX_SHADER,
  GEOMETRY_SHADER,
  TESS_EVALUATION_SHADER,
  TESS_CONTROL_SHADER,
  SHADER_TYPE_COUNT
};

struct ITexture {
  virtual GLuint gl() = 0;
  virtual GLenum getMode() { return GL_TEXTURE_2D; }
};

struct UTexture {

  UTexture() {}
  UTexture(GLuint textureID) { this->textureID = textureID; }
  UTexture(GLuint textureID, GLenum textureMode) {
    this->textureID = textureID;
    this->textureMode = textureMode;
  }

  GLuint textureID = -1;
  GLenum textureMode = GL_TEXTURE_2D;
};

#define UNIFORMS_LIST(o)                                                       \
  o(VEC2, glm::vec2) o(VEC3, glm::vec3) o(VEC4, glm::vec4) o(MAT2, glm::mat2)  \
      o(MAT3, glm::mat3) o(MAT4, glm::mat4) o(FLOAT, float) o(BOOL, bool)      \
          o(INT, int) o(ITEXTURE, ITexture *) o(VEC3PTR, const glm::vec3 *)    \
              o(INTPTR, int *) o(FLOATPTR, float *)                            \
                  o(VEC2PTR, const glm::vec2 *) o(UTEXTURE, UTexture)

enum UniformType {
#define UNIFORMS_ENUMS_DECLARATION(v, T) v,
  UNIFORMS_LIST(UNIFORMS_ENUMS_DECLARATION)
#undef UNIFORMS_ENUMS_DECLARATION
};

struct Uniform {
  union {
#define UNIFORMS_UNION_DECLARATION(v, T) T v;
    UNIFORMS_LIST(UNIFORMS_UNION_DECLARATION)
#undef UNIFORMS_UNION_DECLARATION
  };

  UniformType type;
  int count = 1;

  Uniform() {}
#define UNIFORMS_CONSTRUCTOR(v, T)                                             \
  Uniform(T _##v) : v(_##v), type(UniformType::v) {}
  UNIFORMS_LIST(UNIFORMS_CONSTRUCTOR)
#undef UNIFORMS_CONSTRUCTOR
#define UNIFORMS_CONSTRUCTOR(v, T)                                             \
  Uniform(T _##v, int _count) : v(_##v), type(UniformType::v), count(_count) {}
  UNIFORMS_LIST(UNIFORMS_CONSTRUCTOR)
#undef UNIFORMS_CONSTRUCTOR

  bool bind(GLuint program, GLuint glUniformID) const;
};

struct Program;
struct Material;

struct Shader {
  ResourceHandler file;
  bool use(GLint type);

private:
  GLuint gl_shader = -1;
  friend Program;
};

struct Program {
  Shader *shaders[SHADER_TYPE_COUNT] = {0};
  bool hint_skybox = false;

  void use();
  void bind(Material *material);
  void bind(const char *uniformName, Uniform value);

  inline int getCompilationCount() const { return compilationCount; }
  GLuint gl();

private:
  int compilationCount = 0;
  bool errored = false;
  GLuint gl_shaderProgram = -1;
};

struct VertexAttribute {
  int index;
  int size;
  int attributeDivisor = 0;
  float *sourceData;
  int stride;
};

struct VertexBuffer : public Updatable {
  simple_vector<uint8_t> vertexBuffer;
  simple_vector<VertexAttribute> attributes;

  bool hint_allocation_dynamic = false;
  bool hint_allocation_stream = false;
  int vertexSize() const;

  GLuint gl();
  void use();

private:
  mutable int _vertexsize = -1;
  GLuint gl_vbo = -1;
};

struct IndexBuffer : public Updatable {
  simple_vector<Standard::meshIndex> indexBuffer;
  void use();
  GLuint gl();

private:
  GLuint gl_ebo = -1;
};

struct Mesh {
  VertexBuffer *vbo = nullptr;
  IndexBuffer *ebo = nullptr;

  bool invertedFaces = false;
  int vertexCount();

  void use();

  VertexAttribute getAttribute(int attribIndex);
};

struct Material : public EngineResource {

#define UNIFORMS_FUNC_DECLARATION(v, T)                                        \
  inline Uniform &set(const std::string &name, T a) {                          \
    Uniform val;                                                               \
    val.v = a;                                                                 \
    val.type = UniformType::v;                                                 \
    return uniforms[name] = val;                                               \
  }

  UNIFORMS_LIST(UNIFORMS_FUNC_DECLARATION)
#undef UNIFORMS_FUNC_DECLARATION

  // Uniforms collection
  std::unordered_map<std::string, Uniform> uniforms;
  bool has(const std::string &uniformName);

  void setSetupProgram(Program *program);

  void addMaterial(Material *);
  void popNextMaterial();

  io_buffer serialize() override;
  void deserialize(io_buffer buffer) override;

  // Various hints

  bool hint_isCamera = false;

protected:
  friend Program;
  virtual void bind(Program *program) {}
  simple_vector<Material *> childMaterials;

private:
  // This is for cretaing a uniform collection template from a shader
  Program *setupProgram = nullptr;
  int setupProgramCompilationCount = 0;
};

#undef UNIFORMS_LIST

struct Node : public Material, public EngineComponent<Node> {
  Node *parentNode = nullptr;
  std::list<Node *> children;
  glm::mat4 transformMatrix = glm::mat4(1.0);

  mutable glm::mat4 combinedMatrix;
  mutable bool clean = false;
  mutable bool enableclean = false;

  bool enabled = true;
  bool cachedenabled = true;

public:
  Node();
  bool isEnabled();
  void setEnabled(bool pEnable);
  void setDirty();
  void setParentNode(Node *parent);
  void addChildNode(Node *childNode);
  void setTransformMatrix(const glm::mat4 &newVal);
  void setOffset(glm::vec3 offset);
  void transform(const glm::mat4 &newval);
  const glm::mat4 &getTransformMatrix() const;
  const glm::mat4 &getCombinedMatrix() const;
  void bind(Program *activeProgram) override;
};

struct ModelConfiguration {

  bool depthMask = false;
  bool cullFrontFace = false;
  GLuint renderMode = GL_TRIANGLES;
  GLuint polygonMode = GL_FILL;
  int pointSize = 5;
  int lineWidth = 5;

  uint32_t skipRenderMask = 0;
};

struct Model : public ModelConfiguration, public DrawCallArgs {
  Program *program = nullptr;
  Mesh *mesh = nullptr;
  Material *material = nullptr;
  Node *node = nullptr;
  int zIndex = 0;

  int hint_class = 0;
  bool hint_raycast = false;
  bool hint_editor = false;
  bool hint_selectionpass = false;
  int hint_modelid = 0;
  Material *hint_selection_material = nullptr;

  bool operator<(const Model &model) const;

  virtual void draw();
  bool ready() const;

  bool isEnabled();

  Node *getNode();
  void setNode(Node *node);
};

struct StepInfo {
  Ray mouseRay;
};

struct LogicComponent : EngineComponent<LogicComponent> {

  Material *hint_is_material = nullptr;

  virtual void step(StepInfo info) {}
  virtual void editorStep(StepInfo info) {}
  virtual void editorRender() {}
  virtual void setName(const char *name) override;
  void add(Model *model);

private:
  shambhala::Node *rootNode = nullptr;
};

struct ModelList {
  simple_vector<Model *> models;

  void add(Model *model);
  void remove(Model *model);
  void forceSorting();

  void use();
  int size() const;
  Model *get(int index) const;
  const std::vector<int> &getRenderOrder();

private:
  std::vector<int> modelindices;
  bool shouldSort = true;
};

struct TextureResource : public IResource {
  uint8_t *textureBuffer = nullptr;
  int width;
  int height;
  int components = 3;
  bool hdrSpace = false;
  virtual io_buffer *read() override;
};

struct Texture : public ITexture, video::TextureDesc {

  bool needsUpdate();
  void addTextureResource(TextureResource *textureData);

  GLuint gl() override;
  GLenum getMode() override { return video::TextureDesc::mode; }

  simple_vector<ResourceHandlerAbstract<TextureResource>> textureData;

private:
  GLuint gl_textureID = -1;
};

enum FrameBufferDescriptorFlags {
  FRAME_BUFFER_NULL = 0,
  USE_RENDER_BUFFER = 1 << 0,
  USE_DEPTH = 1 << 1,
  USE_STENCIL = 1 << 2,
  SEPARATE_DEPTH_STENCIL = 1 << 3,
  ONLY_WRITE_DEPTH = 1 << 4,
  ONLY_READ_DEPTH = 1 << 5,
};

struct FrameBufferAttachmentDefinition {
  video::TextureDesc desc;
  video::TextureFormat format;
};

ENUM_OPERATORS(FrameBufferDescriptorFlags)

struct FrameBuffer;
struct FrameBufferOutput : public ITexture {
  GLuint gl() override;

private:
  FrameBuffer *framebuffer;
  int attachmentIndex;
  friend FrameBuffer;
};

class FrameBuffer {
  FrameBufferDescriptorFlags configuration = FRAME_BUFFER_NULL;
  int bufferWidth = -1, bufferHeight = -1;

  int desiredWidth = -1;
  int desiredHeight = -1;

  void initialize();
  void dispose();
  void resize(int screenWidth, int screenHeight);

  GLuint createDepthStencilBuffer();

  inline bool combinedDepthStencil() const {
    return configuration & USE_DEPTH && configuration & USE_STENCIL &&
           !(configuration & SEPARATE_DEPTH_STENCIL);
  }
  simple_vector<GLuint> colorAttachments;
  simple_vector<FrameBufferAttachmentDefinition> attachmentsDefinition;

  GLuint gl_framebuffer = -1;
  GLuint gl_stencilDepthBuffer = -1;
  GLuint gl_depthBuffer = -1;
  GLuint gl_stencilBuffer = -1;

public:
  FrameBuffer();
  FrameBufferOutput *getOutputTexture(int index);
  GLuint getOutputAttachment(int index);
  void addOutputAttachment(FrameBufferAttachmentDefinition def);
  void addOutput(video::TextureFormat format);

  void begin(int width, int height);
  void begin();
  void end();

  void setConfiguration(FrameBufferDescriptorFlags flags);

  inline void setWidth(int width) { desiredWidth = width; }
  inline void setHeight(int height) { desiredHeight = height; }
  int getWidth();
  int getHeight();

  glm::vec4 clearColor;
};

struct RenderShot {
  int frame = 0;
  bool isRoot = false;
  simple_vector<ModelList *> scenes;
};

struct RenderCamera;
struct RenderCameraOutput : public ITexture {
  GLuint gl() override;

private:
  RenderCamera *camera;
  int attachmentIndex;
  friend RenderCamera;
};

struct RenderCamera : public Material, public FrameBuffer {

  RenderCameraOutput *renderOutput(int attachmentIndex);
  virtual void render();

private:
  int currentFrame = -1;
  int boundModelList = 0;
  friend RenderCameraOutput;
};

struct PostProcessCamera : public RenderCamera {
  PostProcessCamera(Program *postprocess);
  PostProcessCamera(const char *shader);

  void render() override;

private:
  Program *postProcessProgram = nullptr;
};

using RenderCamera = RenderCamera;

struct RenderConfiguration {
  int mssaLevel = 0;
  bool wireRendering = false;
  glm::vec3 clearColor = glm::vec3(0.0);
  int virtualWidth = 0;
  int virtualHeight = 0;
};

struct EngineControllers {
  shambhala::ISerializer *serializer = nullptr;
  shambhala::IViewport *viewport = nullptr;
  shambhala::IIO *io = nullptr;
  shambhala::ILogger *logger = nullptr;
  shambhala::audio::IAudio *audio = nullptr;
  shambhala::video::IVideo *video = nullptr;
};

} // namespace shambhala

namespace shambhala {

Node *createNode();
Node *createNode(const char *componentName);
Node *createNode(Node *);
Node *createNode(const char *componentName, Node *copyNode);

Texture *createTexture();
Model *createModel();
Mesh *createMesh();
Program *createProgram();
Shader *createShader();
FrameBuffer *createFramebuffer();
Material *createMaterial();
ModelList *createModelList();
VertexBuffer *createVertexBuffer();
IndexBuffer *createIndexBuffer();
RenderCamera *createRenderCamera();
StepInfo getStepInfo();
const simple_vector<Material *> &getWorldMaterials();

Node *getRootNode();

void disposeModelList(ModelList *list);

// DeclarativeRenderer
void pushMaterial(Material *mat);
void popMaterial();

ModelList *getWorkingModelList();
void setWorkingModelList(ModelList *modelList);
void addModel(Model *model);
void removeModel(Model *model);
void destroyModel(Model *model);

void buildSortPass();

// Controllers
ISerializer *serializer();
IViewport *viewport();
IIO *io();
audio::IAudio *aud();
video::IVideo *vid();

void createEngine(EngineControllers controllers);
void *createWindow(const WindowConfiguration &configuration);
void setActiveWindow(void *window);
void destroyEngine();

void rendertarget_prepareRender();
void updateViewport();
void beginViewport();
void endViewport();

void loop_begin();
void loop_io_sync_step();
void loop_componentUpdate();
void loop_beginRenderContext(int frame);
void loop_endRenderContext();
void loop_beginUIContext();
void loop_endUIContext();
bool loop_shouldClose();
void loop_end();

DrawCallArgs getDefaultArgs();
void drawCall();
void renderPass();

void engine_clearState();
void engine_prepareRender();
void engine_prepareDeclarativeRender();

void hint_selectionpass();
void addComponent(LogicComponent *component);

LogicComponent *getComponent(int index);
int componentCount();

bool input_mouse_free();
} // namespace shambhala

namespace shambhala {
namespace loader {

#include <unordered_map>

using Key = unsigned long;
template <typename T, typename Container> struct LoaderMap {
  std::unordered_map<Key, T *> cache;
  simple_vector<T *> linearCache;
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
    linearCache.push(result);
    countMap[result] = 1;
    return result;
  }

  void unload(T *ptr) {
    int count = --countMap[ptr];
    if (count < 1) {
      delete ptr;
      buildVector();
    }
  }

  void buildVector() {
    linearCache.resize(cache.size());
    int count = 0;
    for (auto &it : cache) {
      linearCache[count++] = it.second;
    }
  }
};

Key computeKey(const char *);
Shader *loadShader(const char *path);
Shader *loadShader(IResource *resource);
Program *loadProgram(IResource *fs, IResource *vs);
Program *loadProgram(const char *fragmentShader, const char *vertexShader);
Texture *loadTexture(const char *path, int channelCount);

void unloadProgram(Program *program);
void unloadTexture(Texture *texture);

Program *getProgram(int index);
int programCount();

Shader *getShader(int index);
int shaderCount();

} // namespace loader
} // namespace shambhala
#undef ENUM_OPERATORS
