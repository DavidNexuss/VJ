#pragma once
#include "adapters/io.hpp"
#include "adapters/log.hpp"
#include "adapters/viewport.hpp"
#include "core/core.hpp"
#include "simple_vector.hpp"
#include <glm/glm.hpp>
#include <list>
#include <memory>
#include <standard.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#pragma once
#define ENUM_OPERATORS(T)                                                      \
  inline T operator~(T a) { return (T) ~(int)a; }                              \
  inline T operator|(T a, T b) { return (T)((int)a | (int)b); }                \
  inline T operator&(T a, T b) { return (T)((int)a & (int)b); }                \
  inline T operator^(T a, T b) { return (T)((int)a ^ (int)b); }                \
  inline T &operator|=(T &a, T b) { return (T &)((int &)a |= (int)b); }        \
  inline T &operator&=(T &a, T b) { return (T &)((int &)a &= (int)b); }        \
  inline T &operator^=(T &a, T b) { return (T &)((int &)a ^= (int)b); }

using uint32_t = unsigned int;
using uint8_t = unsigned char;

namespace shambhala {

struct UIComponent {
  bool uiRender = false;
  bool uiSelected = false;
};

template <typename T> struct EngineComponent : public UIComponent {
  inline static int indexCount = 0;

  std::string stringName;
  int indexName = 0;

  EngineComponent() { indexName = indexCount++; }

  EngineComponent(const EngineComponent &other) {
    indexName = indexCount++;
    other.stringName = other.stringName + " " + std::to_string(indexName);
  }
  std::string &getName() {
    if (stringName.empty())
      stringName = std::to_string(indexName);
    return stringName;
  }

  void setName(const char *name) {
    if (name != nullptr)
      stringName = name;
  }
};
struct IResource {
  virtual io_buffer *read() = 0;
  const char *resourcename = nullptr;
  bool claim();
  bool needsUpdate();
  void forceUpdate();

private:
  bool _needsUpdate = true;
};

struct MemoryResource : public IResource {
  io_buffer *buffer;
  virtual io_buffer *read() override;
};

struct Shader {
  GLuint shader = -1;
  IResource *file = nullptr;
};

enum ShaderType {
  FRAGMENT_SHADER = 0,
  VERTEX_SHADER,
  GEOMETRY_SHADER,
  TESS_EVALUATION_SHADER,
  TESS_CONTROL_SHADER,
  SHADER_TYPE_COUNT
};

struct Program {
  Shader shaders[SHADER_TYPE_COUNT];
  GLuint shaderProgram = -1;

  bool hint_skybox = false;
  bool errored = false;
};

struct UTexture {
  GLuint texID = -1;
  int unit = 0;
  GLenum mode = GL_TEXTURE_2D;
  UTexture() {}
  UTexture(GLuint _texID, int _unit) : texID(_texID), unit(_unit) {}
  UTexture(GLuint _texID, int _unit, GLenum _mode)
      : texID(_texID), unit(_unit), mode(_mode) {}
};

struct Texture;
struct DynamicTexture {
  Texture *sourceTexture = nullptr;
  int unit = 0;

  DynamicTexture() {}
  DynamicTexture(Texture *sourceTexture) {
    this->sourceTexture = sourceTexture;
  }
};

#define UNIFORMS_LIST(o)                                                       \
  o(VEC2, glm::vec2) o(VEC3, glm::vec3) o(VEC4, glm::vec4) o(MAT2, glm::mat2)  \
      o(MAT3, glm::mat3) o(MAT4, glm::mat4) o(FLOAT, float) o(BOOL, bool)      \
          o(INT, int) o(SAMPLER2D, UTexture) o(VEC3PTR, const glm::vec3 *)     \
              o(BINDLESS_TEXTURE, GLuint64) o(DYNAMIC_TEXTURE, DynamicTexture)

enum UniformType {
#define UNIFORMS_ENUMS_DECLARATION(v, T) v,
  UNIFORMS_LIST(UNIFORMS_ENUMS_DECLARATION)
#undef UNIFORMS_ENUMS_DECLARATION
};

class Uniform {
public:
  union {
#define UNIFORMS_UNION_DECLARATION(v, T) T v;
    UNIFORMS_LIST(UNIFORMS_UNION_DECLARATION)
#undef UNIFORMS_UNION_DECLARATION
  };

  UniformType type;
  mutable bool dirty;
  int count = 1;

  Uniform() {}
#define UNIFORMS_CONSTRUCTOR(v, T)                                             \
  Uniform(T _##v) : v(_##v), type(UniformType::v), dirty(true) {}
  UNIFORMS_LIST(UNIFORMS_CONSTRUCTOR)
#undef UNIFORMS_CONSTRUCTOR
#define UNIFORMS_CONSTRUCTOR(v, T)                                             \
  Uniform(T _##v, int _count)                                                  \
      : v(_##v), type(UniformType::v), dirty(true), count(_count) {}
  UNIFORMS_LIST(UNIFORMS_CONSTRUCTOR)
#undef UNIFORMS_CONSTRUCTOR

  inline void setDirty() { dirty = true; }
  bool bind(GLuint glUniformID) const;
};

struct Texture;

struct VertexAttribute {
  int index;
  int size;
  int attributeDivisor = 0;
  float *sourceData;
  int stride;
};

struct VertexBuffer {
  simple_vector<uint8_t> vertexBuffer;
  simple_vector<VertexAttribute> attributes;
  GLuint vbo = -1;

  bool hint_allocation_dynamic = false;
  bool hint_allocation_stream = false;
  bool updateData = false;
  int vertexSize() const;

private:
  mutable int _vertexsize = -1;
};

struct IndexBuffer {
  simple_vector<Standard::meshIndex> indexBuffer;
  GLuint ebo = -1;
  bool updateData = false;
};

struct Mesh {
  // int meshLayout = 0;
  VertexBuffer *vbo = nullptr;
  IndexBuffer *ebo = nullptr;

  bool invertedFaces = false;
  int vertexCount();

  VertexAttribute getAttribute(int attribIndex);
};

struct Material {

#define UNIFORMS_FUNC_DECLARATION(v, T)                                        \
  inline void set(const std::string &name, T a) {                              \
    Uniform val;                                                               \
    val.v = a;                                                                 \
    val.dirty = true;                                                          \
    val.type = UniformType::v;                                                 \
    uniforms[name] = val;                                                      \
  }

  UNIFORMS_LIST(UNIFORMS_FUNC_DECLARATION)
#undef UNIFORMS_FUNC_DECLARATION

  std::unordered_map<std::string, Uniform> uniforms;
  VertexBuffer *vbo = nullptr;

  virtual void update(float deltatime) {}
  virtual void bind(Program *activeProgram) {}
  bool needsFrameUpdate = false;
  bool hasCustomBindFunction = false;
  int currentFrame = -1;

  simple_vector<Material *> childMaterials;

  void addMaterial(Material *);
  void popNextMaterial();
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
  void transform(const glm::mat4 &newval);
  const glm::mat4 &getTransformMatrix() const;
  const glm::mat4 &getCombinedMatrix() const;
  void bind(Program *activeProgram) override;
};

struct ModelConfiguration {

  bool enabled = true;
  bool depthMask = false;
  bool cullFrontFace = false;
  GLuint renderMode = GL_TRIANGLES;
  GLuint polygonMode = GL_FILL;
  int pointSize = 10;
  int lineWidth = 10;

  uint32_t skipRenderMask = 0;

  int zIndex = 0;
};

struct Model : public ModelConfiguration {
  Program *program = nullptr;
  Mesh *mesh = nullptr;
  Material *material = nullptr;
  Node *node = nullptr;

  bool operator<(const Model &model) const;
  void draw();
  bool ready() const;

  Model *createInstance();
};

struct ModelList {
  simple_vector<Model *> models;

  void add(Model *model);
  void forceSorting();

  int size() const;
  Model *get(int index) const;
  const std::vector<int> &getRenderOrder();

  ModelList *createInstance();

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
struct Texture {
  simple_vector<TextureResource *> textureData;

  GLenum textureMode = GL_TEXTURE_2D;
  GLuint _textureID = -1;

  bool needsUpdate();
  void addTextureResource(TextureResource *textureData);
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

ENUM_OPERATORS(FrameBufferDescriptorFlags)

struct FrameBufferAttachmentDescriptor {
  GLuint internalFormat;
  GLuint externalFormat;
  GLenum type;
  bool useNeareast;
};

class FrameBuffer {
  FrameBufferDescriptorFlags configuration = FRAME_BUFFER_NULL;
  GLuint _framebuffer = -1;

  int bufferWidth = -1, bufferHeight = -1;

  void initialize();
  void dispose();
  void resize(int screenWidth, int screenHeight);

  GLuint createDepthStencilBuffer();

  inline bool combinedDepthStencil() const {
    return configuration & USE_DEPTH && configuration & USE_STENCIL &&
           !(configuration & SEPARATE_DEPTH_STENCIL);
  }

public:
  void addChannel(const FrameBufferAttachmentDescriptor &fbodef);
  void begin(int width, int height);
  void end();

  GLuint stencilDepthBuffer;

  GLuint depthBuffer;
  GLuint stencilBuffer;

  vector<GLuint> colorAttachments;
  vector<FrameBufferAttachmentDescriptor> attachmentsDefinition;

  void setConfiguration(FrameBufferDescriptorFlags flags);
  int getWidth();
  int getHeight();
};

struct RenderCamera;
struct RenderBinding {
  RenderCamera *renderCamera = nullptr;
  int attachmentIndex;
  const char *uniformAttribute;
};

struct RenderCamera;

struct RenderShot {
  int currentFrame = 0;
  bool isRoot = false;
  simple_vector<ModelList *> scenes;

  void updateFrame();
};

struct IRenderable {
  virtual RenderCamera *render(const RenderShot &shot) = 0;
};

struct RenderCamera : public Material, IRenderable {
  FrameBuffer *frameBuffer = nullptr;
  int modelListInput = 0;

  simple_vector<RenderBinding> renderBindings;
  simple_vector<RenderCamera *> dummyInput;

  Program *overrideProgram = nullptr;
  Program *postprocessProgram = nullptr;

  RenderCamera();

  void beginRender(const RenderShot &shot);
  void endRender(const RenderShot &shot);

  virtual RenderCamera *render(const RenderShot &shot) override;
  virtual void bind(Program *activeProgram) override;

  void addInput(RenderCamera *child, int attachmentIndex,
                const char *uniformAttribute);
  void addDummyInput(RenderCamera *child);
  RenderCamera *getDummyInput(int index);
  int getDummyInputCount();

  void addOutput(FrameBufferAttachmentDescriptor desc);
  void setFrameBufferConfiguration(FrameBufferDescriptorFlags);
  int getWidth();
  int getHeight();
  void setSize(int width, int height);
  void setModelList(int index);

private:
  int width = 0;
  int height = 0;
};

struct RenderConfiguration {
  int mssaLevel = 0;
  bool wireRendering = false;
  glm::vec3 clearColor = glm::vec3(0.0);
  int virtualWidth = 0;
  int virtualHeight = 0;
};

struct EngineControllers {
  shambhala::IViewport *viewport;
  shambhala::IIO *io;
  shambhala::ILogger *logger;
};

struct DeviceParameters {
  int maxTextureUnits;
};

struct EngineParameters : public EngineControllers {};
} // namespace shambhala
namespace shambhala {

namespace device {

GLuint compileShader(const char *data, GLenum type, const char *resourcename);
GLuint compileProgram(GLuint *shaders, GLint *status);
GLuint createVAO();
GLuint createVBO(const simple_vector<uint8_t> &vertexBuffer, GLuint *vbo);
GLuint createEBO(const simple_vector<Standard::meshIndex> &indexBuffer,
                 GLuint *ebo);
GLuint createTexture(bool filter);
GLuint createCubemap();
GLuint createRenderBuffer();
GLuint createFramebuffer();
GLuint createRenderBuffer();
GLuint getShaderType(int shaderType);
GLuint getUniform(GLuint program, const char *name);

void uploadTexture(GLenum target, unsigned char *texturebuffer, int width,
                   int height, int components, bool hdr);

void uploadDepthTexture(GLuint texture, int width, int height);
void uploadStencilTexture(GLuint texture, int width, int height);
void uploadDepthStencilTexture(GLuint texture, int width, int height);

void configureRenderBuffer(GLenum mode, int width, int height);
void disposeShader(GLuint shader);
void disposeProgram(GLuint program);
void disposeTexture(GLuint texture);
void disposeVao(GLuint vao);
void disposeVbo(GLuint vbo);
void bindAttribute(GLuint vbo, int index, int size, int stride, int offset,
                   int divisor);
void bindProgram(GLuint program);
void bindVao(GLuint vao);
void bindVbo(GLuint vbo);
void bindEbo(GLuint ebo);
void bindTexture(GLuint textureId, GLenum mode);
void bindTexture(GLuint textureId, GLenum mode, int textureUnit);
void bindRenderBuffer(GLuint renderBuffer);
void bindFrameBuffer(GLuint frameBuffer);
void cullFrontFace(bool frontFace);

void useModelConfiguration(ModelConfiguration *configuration);
void useTexture(UTexture texture);
void useTexture(DynamicTexture texture);
void useTexture(Texture *texture);
void useProgram(Program *program);
void useVertexBuffer(VertexBuffer *vertexbuffer);
void useIndexBuffer(IndexBuffer *indexBuffer);
void useMesh(Mesh *mesh);
void useMaterial(Material *material);
void useUniform(const char *name, const Uniform &value);
void useModelList(ModelList *modelList);

void ignoreProgramBinding(bool ignore);

void drawCall();
void renderPass();
DeviceParameters queryDeviceParameters();
} // namespace device

Node *createNode();
Node *createNode(const char *componentName);
Node *createNode(Node *);
Node *createNode(const char *componentName, Node *copyNode);

Texture *createTexture();
Model *createModel();
Mesh *createMesh();
Program *createProgram();
FrameBuffer *createFramebuffer();
Material *createMaterial();
ModelList *createModelList();
RenderCamera *createRenderCamera();
VertexBuffer *createVertexBuffer();
IndexBuffer *createIndexBuffer();

Node *getRootNode();

void disposeModelList(ModelList *list);

// DeclarativeRenderer
void setWorldMaterial(int clas, Material *worldMaterial);
void addWorldMaterial(Material *worldMaterial);
void removeWorldMaterial(Material *worldMaterial);
void pushRenderConfiguration(RenderConfiguration *configuration);
void popRenderConfiguration();

ModelList *getWorkingModelList();
void setWorkingModelList(ModelList *modelList);
void addModel(Model *model);

void buildSortPass();

// Controllers
IViewport *viewport();
IIO *io();

void createEngine(EngineParameters parameters);
void *createWindow(const WindowConfiguration &configuration);
void setActiveWindow(void *window);
void destroyEngine();

void rendertarget_prepareRender();
void updateViewport();
void beginViewport();
void endViewport();
void loop_beginRenderContext();
void loop_endRenderContext();
void loop_beginUIContext();
void loop_endUIContext();
bool loop_shouldClose();

void engine_clearState();
void engine_prepareRender();
void engine_prepareDeclarativeRender();
} // namespace shambhala

namespace shambhala {
namespace loader {

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

Key computeKey(const char *);
Program *loadProgram(const char *fragmentShader, const char *vertexShader);
Texture *loadTexture(const char *path);

void unloadProgram(Program *program);
void unloadTexture(Texture *texture);

} // namespace loader
} // namespace shambhala
#undef ENUM_OPERATORS
