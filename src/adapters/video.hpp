#pragma once
#include "core/core.hpp"
#include "standard.hpp"
#include <string>

namespace video {

struct DrawCallArgs {
  int instanceCount = 0;
  union {
    int vertexCount = 0;
    int indexCount;
  };
  bool indexed = false;
  bool frontCulled = false;

  GLenum drawMode = GL_TRIANGLES;
};

struct VideoDeviceParameters {
  int maxTextureUnits;
};

struct TextureDesc {
  bool clamp = false;
  GLenum mode = GL_TEXTURE_2D;
  GLenum minFilter = GL_NEAREST;
  GLenum magFilter = GL_NEAREST;
};

struct TextureFormat {
  GLuint internalFormat = -1;
  GLuint externalFormat = -1;
  GLenum type = -1;
};

struct TextureUploadDesc {
  GLuint textureID = -1;
  TextureFormat format;
  GLenum target = GL_TEXTURE_2D;

  uint8_t *buffer = nullptr;
  int width = -1;
  int height = -1;

  bool isDepth = false;
  bool isStencil = false;
  bool isDepthStencil = false;
};

struct FrameBufferDesc {
  GLuint *attachments = nullptr;
  int attachmentCount = -1;
  GLuint depthAttachment = -1;
  GLuint stencilAttachment = -1;
  GLuint depthStencilAttachment = -1;
  GLuint renderBufferAttachment = -1;

  GLuint oldFramebuffer = -1;
};

struct ShaderDesc {
  const char *data = nullptr;
  const char *name = nullptr;
  GLenum type = -1;
};

struct ProgramDesc {
  GLuint *shaders = nullptr;
  int shaderCount = -1;
};

struct BufferDesc {
  GLenum type = -1;
};

struct BufferUploadDesc {
  uint8_t *buffer = nullptr;
  int size = -1;
  int start = -1;
  GLuint id = -1;
  GLenum mode = GL_STATIC_DRAW;
};

struct AttributeDesc {
  GLuint buffer = -1;
  int index = -1;
  int size = -1;
  int stride = -1;
  int offset = -1;
  int divisor = -1;
};

struct ProgramStatus {
  bool errored;
  std::string errorMsg;
};

struct ShaderStatus {
  bool errored;
  std::string errorMsg;
};

struct RenderBufferDesc {
  GLenum format = -1;
  int width = -1;
  int height = -1;
};

union ConfigurationValue {
  GLuint val;
  glm::vec2 vec2;
  glm::vec3 vec3;
  glm::vec4 vec4;

  ConfigurationValue() {}
  ConfigurationValue(GLuint p) { val = p; }
  ConfigurationValue(glm::vec2 p) { vec2 = p; }
  ConfigurationValue(glm::vec3 p) { vec3 = p; }
  ConfigurationValue(glm::vec4 p) { vec4 = p; }
};

// This should be supported by the driver
const static int SH_CULL_FACE_MODE_BACK = -1;
const static int SH_CLEAR_COLOR = -2;

enum UniformType {
  SH_UNIFORM_INT = 0,
  SH_UNIFORM_FLOAT,
  SH_UNIFORM_VEC2,
  SH_UNIFORM_VEC3,
  SH_UNIFORM_VEC4,
  SH_UNIFORM_MAT2,
  SH_UNIFORM_MAT3,
  SH_UNIFORM_MAT4,
  SH_UNIFORM_SAMPLER,
};

/** ENGINE CLEAN UP HOOKS **/

void initDevice();
void enableDebug(bool);

void stepBegin();
void stepEnd();

/** DRIVER STATUS **/

ProgramStatus &statusProgramCompilation();
ShaderStatus &statusShaderCompilation();

/** CREATE AND COMPILE FUNCTIONS **/

// Shaders
GLuint compileShader(ShaderDesc);
GLuint compileProgram(ProgramDesc);

// Buffers
GLuint createBuffer(BufferDesc);
GLuint createTexture(TextureDesc);
GLuint createFramebuffer(FrameBufferDesc);
GLuint createRenderbuffer(RenderBufferDesc);

void uploadTexture(TextureUploadDesc);
void uploadBuffer(BufferUploadDesc);
GLuint getUniform(GLuint program, const char *name);

/** DISPOSE FUNCTIONS **/

void disposeShader(GLuint shader);
void disposeProgram(GLuint program);
void disposeTexture(GLuint texture);
void disposeBuffer(GLuint bo);

/** BIND FUNCTIONS **/

void bindAttribute(AttributeDesc);

void bindProgram(GLuint program);
void bindVao(GLuint vao);
void bindBuffer(GLuint buffer);
void bindFrameBuffer(GLuint frameBuffer);
void bindUniform(GLuint program, GLuint id, GLenum type, void *value,
                 int count = 1);

/**MISC **/
VideoDeviceParameters queryDeviceParameters();
void setViewport(int width, int height);

/** DRAW **/
void clear(GLenum flags);
void drawCall(DrawCallArgs args);

/** DRAW STATE **/

void set(GLenum property, ConfigurationValue value);

video::BufferUploadDesc descUpload(span, GLuint buffer);
video::TextureDesc descDepthTexture();
video::TextureDesc descStencilTexture();
video::TextureUploadDesc descDepthUpload(int width, int height, GLuint texture);
video::TextureUploadDesc descStencilUpload(int width, int height,
                                           GLuint texture);

video::TextureFormat descTextureFormat(bool hdr, int components);
} // namespace video
