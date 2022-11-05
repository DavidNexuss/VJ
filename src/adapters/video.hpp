#pragma once
#include "core/core.hpp"
#include "standard.hpp"
#include <string>

namespace shambhala {

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

namespace video {

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

struct IVideo {

  /** ENGINE CLEAN UP HOOKS **/

  virtual void initDevice() = 0;
  virtual void enableDebug(bool) = 0;

  virtual void stepBegin() = 0;
  virtual void stepEnd() = 0;

  /** DRIVER STATUS **/

  virtual ProgramStatus &statusProgramCompilation() = 0;
  virtual ShaderStatus &statusShaderCompilation() = 0;

  /** CREATE AND COMPILE FUNCTIONS **/

  // Shaders
  virtual GLuint compileShader(ShaderDesc) = 0;
  virtual GLuint compileProgram(ProgramDesc) = 0;

  // Buffers
  virtual GLuint createBuffer(BufferDesc) = 0;
  virtual GLuint createTexture(TextureDesc) = 0;
  virtual GLuint createFramebuffer(FrameBufferDesc) = 0;
  virtual GLuint createRenderbuffer(RenderBufferDesc) = 0;

  virtual void uploadTexture(TextureUploadDesc) = 0;
  virtual void uploadBuffer(BufferUploadDesc) = 0;
  virtual GLuint getUniform(GLuint program, const char *name) = 0;

  /** DISPOSE FUNCTIONS **/

  virtual void disposeShader(GLuint shader) = 0;
  virtual void disposeProgram(GLuint program) = 0;
  virtual void disposeTexture(GLuint texture) = 0;
  virtual void disposeBuffer(GLuint bo) = 0;

  /** BIND FUNCTIONS **/

  virtual void bindAttribute(AttributeDesc) = 0;

  virtual void bindProgram(GLuint program) = 0;
  virtual void bindVao(GLuint vao) = 0;
  virtual void bindBuffer(GLuint buffer) = 0;
  virtual void bindFrameBuffer(GLuint frameBuffer) = 0;
  virtual void bindUniform(GLuint program, GLuint id, GLenum type, void *value,
                           int count = 1) = 0;

  /**MISC **/
  virtual VideoDeviceParameters queryDeviceParameters() = 0;
  virtual void setViewport(int width, int height) = 0;

  /** DRAW **/
  virtual void clear(GLenum flags) = 0;
  virtual void drawCall(DrawCallArgs args) = 0;

  /** DRAW STATE **/

  virtual void set(GLenum property, ConfigurationValue value) = 0;
};
} // namespace video

} // namespace shambhala

#include <simple_vector.hpp>

namespace shambhala {
video::BufferUploadDesc descUpload(simple_vector_span, GLuint buffer);
video::TextureDesc descDepthTexture();
video::TextureDesc descStencilTexture();
video::TextureUploadDesc descDepthUpload(int width, int height, GLuint texture);
video::TextureUploadDesc descStencilUpload(int width, int height,
                                           GLuint texture);

video::TextureFormat descTextureFormat(bool hdr, int components);
} // namespace shambhala
