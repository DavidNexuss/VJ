#pragma once
#include "controllers/io.hpp"
#include "controllers/viewport.hpp"
#include "core/core.hpp"
#include "simple_vector.hpp"
#include <glm/glm.hpp>
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

struct IResource {
  virtual io_buffer read();
  bool needsUpdate;
};

struct MemoryResource : public IResource {
  io_buffer buffer;
  virtual io_buffer read();
};

struct Node {
  Node *parentNode = nullptr;
  simple_vector<Node *> children;
  glm::mat4 transformMatrix = glm::mat4(1.0);

  mutable glm::mat4 combinedMatrix;
  mutable bool clean;

  void removeChildNode(Node *childNode);

public:
  Node();
  void setDirty();
  void addChildNode(Node *childNode);
  void setTransformMatrix(const glm::mat4 &newVal);
  const glm::mat4 &getTransformMatrix() const;
  const glm::mat4 &getCombinedMatrix() const;
};

struct Shader {
  GLuint shader;
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
};

struct UTexture {
  GLuint texID;
  int unit;
  GLenum mode = GL_TEXTURE_2D;
  UTexture(GLuint _texID, int _unit) : texID(_texID), unit(_unit) {}
  UTexture(GLuint _texID, int _unit, GLenum _mode)
      : texID(_texID), unit(_unit), mode(_mode) {}
};

struct Texture;
struct DynamicTexture {
  Texture *sourceTexture;
  int unit;
  GLenum mode = GL_TEXTURE_2D;

  DynamicTexture();
  DynamicTexture(Texture *sourceTexture, int unit);
  DynamicTexture(Texture *sourceTexture, int unit, GLenum mode);
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
};

struct WorldMaterial {
  virtual void update(float deltatime) {}
  virtual void bind(Program *activeProgram) = 0;
  bool needsFrameUpdate;
};

#undef UNIFORMS_LIST

struct MeshLayout {
  struct MeshLayoutAttribute {
    int index;
    int size;
  };
  simple_vector<MeshLayoutAttribute> attributes;
  int vertexSize() const;
  GLuint vao = -1;

private:
  mutable int _vertexsize = -1;
};

struct Mesh {
  simple_vector<uint8_t> vertexBuffer;
  simple_vector<unsigned short> indexBuffer;
  MeshLayout *meshLayout;

  GLuint vbo = -1;
  GLuint ebo = -1;
  bool needsVBOUpdate = false;
  bool needsEBOUpdate = false;
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
};

struct ModelList {
  simple_vector<Model *> models;
  void add(Model *model);
  void forceSorting();
  const std::vector<int> &getRenderOrder();

private:
  std::vector<int> modelindices;
  bool shouldSort;
};

struct TextureResource : public IResource {
  uint8_t *textureBuffer;
  int width;
  int height;
  int components;
};
struct Texture {
  simple_vector<TextureResource *> textureData;

  bool isCubemap = false;
  GLuint textureUnit = 0;
  GLenum textureMode = GL_TEXTURE_2D;
  bool needsTextureUpdate = true;

  GLuint _textureID = -1;

  void addTextureResource(TextureResource *textureData);
};

enum FrameBufferDescriptorFlags {
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
};

class FrameBuffer {
  FrameBufferDescriptorFlags configuration;
  GLuint _framebuffer = -1;

  int bufferWidth = 0, bufferHeight = 0;

  void initialize();
  void dispose();
  void resize(int screenWidth, int screenHeight);
  void check();

  GLuint createDepthStencilBuffer();

  inline bool combinedDepthStencil() const {
    return configuration & USE_DEPTH && configuration & USE_STENCIL &&
           !(configuration & SEPARATE_DEPTH_STENCIL);
  }

public:
  void begin(int width, int height);
  void end();

  GLuint stencilDepthBuffer;

  GLuint depthBuffer;
  GLuint stencilBuffer;

  vector<GLuint> colorAttachments;
  vector<FrameBufferAttachmentDescriptor> attachmentsDefinition;
};

struct EngineControllers {
  shambhala::IViewport *viewport;
  shambhala::IIO *io;
};

struct EngineParameters : public EngineControllers {};

namespace shambhala {

namespace device {

GLuint compileShader(const char *data, GLenum type);
GLuint compileProgram(GLuint *shaders);
GLuint
createVAO(const simple_vector<MeshLayout::MeshLayoutAttribute> &attributes);
GLuint createVBO(const simple_vector<uint8_t> &vertexBuffer, GLuint *vbo);
GLuint createEBO(const simple_vector<unsigned short> &indexBuffer, GLuint *ebo);
GLuint createTexture(bool filter);
GLuint createCubemap();
GLuint createRenderBuffer();
GLuint createFramebuffer();
GLuint createRenderBuffer();
GLuint getShaderType(int shaderType);
GLuint getUniform(GLuint program, const char *name);

void uploadTexture(GLenum target, unsigned char *texturebuffer, int width,
                   int height, int components);

void uploadDepthTexture(GLuint texture, int width, int height);
void uploadStencilTexture(GLuint texture, int width, int height);
void uploadDepthStencilTexture(GLuint texture, int width, int height);

void configureRenderBuffer(GLenum mode, int width, int height);
void disposeShader(GLuint shader);
void disposeProgram(GLuint program);
void disposeTexture(GLuint texture);
void disposeVao(GLuint vao);
void disposeVbo(GLuint vbo);
void bindVao(GLuint vao);
void bindVbo(GLuint vbo);
void bindEbo(GLuint ebo);
void bindTexture(GLuint textureId, GLenum mode);
void bindTexture(GLuint textureId, GLenum mode, int textureUnit);
void bindRenderBuffer(GLuint renderBuffer);
void bindFrameBuffer(GLuint frameBuffer);

void useModelConfiguration(ModelConfiguration *configuration);
void useTexture(UTexture texture);
void useProgram(Program *program);
void useTexture(Texture *texture);
void useMeshLayout(MeshLayout *layout);
void useMesh(Mesh *mesh);
void useMaterial(Material *material);
void clear(int flags);
} // namespace device

Node *createNode();
Texture *createTexture();
Model *createModel();
Mesh *createMesh();
MeshLayout *createMeshLayout();
Program *createProgram();
FrameBuffer *createFramebuffer();
Material *createMaterial();
ModelList *createModelList();

// DeclarativeRenderer
void setWorldMaterial(int clas, WorldMaterial *worldMaterial);
ModelList *getWorkingModelList();
void setWorkingModelList(ModelList *modelList);
void addModel(Model *model);

void buildSortPass();
void renderPass();

// Controllers
IViewport *viewport();
IIO *io();

void createEngine(EngineParameters parameters);
void *createWindow(const WindowConfiguration &configuration);
void setActiveWindow(void *window);
void destroyEngine();

void loop_beginRenderContext();
void loop_endRenderContext();
void loop_beginUIContext();
void loop_endUIContext();
void loop_declarativeRender();
bool loop_shouldClose();

namespace resource {
MemoryResource *ioMemoryFile(const char *path);
TextureResource *stbiTextureMemory(MemoryResource *memoryResource);
TextureResource *stbiTextureFile(const char *path, int desiredChannels);
} // namespace resource

namespace util {
simple_vector<uint8_t> createCube();
Mesh *meshCreateCube();
} // namespace util
} // namespace shambhala

#undef ENUM_OPERATORS
