#include "video_gl.hpp"
#include <adapters/log.hpp>
#include <unordered_map>
#include <vector>

using namespace shambhala;
using namespace shambhala::video;

#define SANITY(x)                                                              \
  { x }

#define EXT_BUFFER_CREATE

static struct {

  ProgramStatus programStatus;
  ShaderStatus shaderStatus;

} driverState;

struct BufferInformation {
  GLenum type;
};

static struct {

  GLuint currentVao = -1;
  GLuint currentProgram = -1;
  GLuint currentFramebuffer = -1;
  GLuint currentTexture = -1;

  // Buffers
  std::unordered_map<int, GLuint> boundAttributes;
  std::unordered_map<GLuint, BufferInformation> buffers;
  std::unordered_map<GLenum, GLuint> boundBuffers;

  // Textures
  std::unordered_map<int, GLuint> boundUnits;

  bool isFrameBufferBound(GLuint buffer) {
    return buffer == currentFramebuffer;
  }

  void bindFrameBuffer(GLuint buffer) { currentFramebuffer = buffer; }

  bool isBufferBound(GLuint buffer) {
    return boundBuffers[buffers[buffer].type] == buffer;
  }

  void bindBuffer(GLuint buffer) {
    boundBuffers[buffers[buffer].type] = buffer;
  }

  BufferInformation &getBufferInformation(GLuint buffer) {
    return buffers[buffer];
  }

  void setBufferInformation(BufferInformation inf, GLuint buffer) {
    buffers[buffer] = inf;
  }

  void bindVao(GLuint vbo) { currentVao = vbo; }
  void bindProgram(GLuint pro) { currentProgram = pro; }

  bool isProgramBound(GLuint program) { return currentProgram == program; }
  bool isVAOBound(GLuint vao) { return currentVao == vao; }

  bool isAttributeBound(int attribute) {
    return boundAttributes[attribute] == boundBuffers[GL_ARRAY_BUFFER];
  }

  void bindAttribute(int attribute) {
    boundAttributes[attribute] = boundBuffers[GL_ARRAY_BUFFER];
  }
} bindState;

static void glError(GLenum source, GLenum type, GLuint id, GLenum severity,
                    GLsizei length, const GLchar *message,
                    const void *userParam) {

  // TODO: Severity magic value
  if (severity >= 37190 &&
      (id == GL_INVALID_OPERATION || type == GL_INVALID_OPERATION)) {

    std::cerr << "[GL " << severity << "] " << message << std::endl;
    throw std::runtime_error{"error"};
  }
}
// ----------------------------------------[DRIVER BEGIN

void OpenGLDriver::enableDebug(bool pEnable) {
  if (pEnable) {
    glDebugMessageCallback(glError, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
  } else {
    glDisable(GL_DEBUG_OUTPUT);
  }
}
void OpenGLDriver::initDevice() {}
void OpenGLDriver::stepBegin() {}
void OpenGLDriver::stepEnd() {}

ProgramStatus &OpenGLDriver::statusProgramCompilation() {
  return driverState.programStatus;
}

ShaderStatus &OpenGLDriver::statusShaderCompilation() {
  return driverState.shaderStatus;
}

GLuint OpenGLDriver::compileShader(ShaderDesc desc) {

  ShaderStatus &pStatus = driverState.shaderStatus;

  auto type = desc.type;
  auto data = desc.data;
  auto name = desc.name;
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &data, NULL);
  glCompileShader(shader);
  GLint compileStatus = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
  pStatus.errored = compileStatus == GL_FALSE;

  if (compileStatus == GL_FALSE) {

    GLint errorSize;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorSize);
    pStatus.errorMsg.resize(errorSize);
    glGetShaderInfoLog(shader, errorSize, &errorSize, &pStatus.errorMsg[0]);
    glDeleteShader(shader);
  }

  return shader;
}

GLuint OpenGLDriver::compileProgram(ProgramDesc desc) {

  ProgramStatus &pStatus = driverState.programStatus;

  if (desc.shaders == 0) {
    pStatus.errored = true;
    pStatus.errorMsg = "[DEVICE] Empty shader list !";
    return -1;
  }
  GLuint program = glCreateProgram();
  for (int i = 0; i < desc.shaderCount; i++) {
    glAttachShader(program, desc.shaders[i]);
  }

  GLint status;
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  GLint InfoLogLength;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);

  if (InfoLogLength > 0) {
    pStatus.errorMsg.resize(InfoLogLength + 1);
    glGetProgramInfoLog(program, InfoLogLength, NULL, &pStatus.errorMsg[0]);
  }

  pStatus.errored = status == GL_FALSE;
  if (status == GL_FALSE) {
    glDeleteProgram(program);
  }

  for (int i = 0; i < desc.shaderCount; i++) {
    glDetachShader(program, desc.shaders[i]);
  }

  return program;
}
GLuint OpenGLDriver::createVAO() {
  GLuint vao;
  glGenVertexArrays(1, &vao);
  return vao;
}
GLuint OpenGLDriver::createBuffer(BufferDesc desc) {
  GLuint buff;
#ifdef EXT_BUFFER_CREATE
  glCreateBuffers(1, &buff);
#else
  glGenBuffers(1, &buff);
#endif
  BufferInformation inf;
  inf.type = desc.type;
  bindState.setBufferInformation(inf, buff);
  return buff;
}

void OpenGLDriver::uploadBuffer(BufferUploadDesc desc) {

  static GLenum mode = GL_STATIC_DRAW;
#ifdef EXT_BUFFER_CREATE
  if (desc.start == 0) {
    glNamedBufferData(desc.id, desc.size, desc.buffer, mode);
  } else {
    glNamedBufferSubData(desc.id, desc.start, desc.size, desc.buffer);
  }
#else
  OpenGLDriver::bindBuffer(desc.id);
  if (desc.start == 0) {
    glBufferData(GL_ARRAY_BUFFER, desc.size, desc.buffer, mode);
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, desc.start, desc.size, desc.buffer);
  }

#endif
}

void OpenGLDriver::bindTexture(GLuint textureId, GLenum target) {
  if (bindState.currentTexture != textureId) {
    glBindTexture(target, textureId);
    bindState.currentTexture = textureId;
  }
}
int OpenGLDriver::bindTexture(GLuint textureId, GLenum target, GLuint unit) {
  if (bindState.boundUnits[unit] != textureId) {
    bindState.boundUnits[unit] = textureId;
    bindTexture(textureId, target);
    glActiveTexture(unit + GL_TEXTURE0);
  }
  return unit;
}

int OpenGLDriver::bindTextureUnit(GLuint textureId, GLenum target) {
  return bindTexture(textureId, target, textureId % 1000);
}

GLuint OpenGLDriver::createTexture(TextureDesc desc) {

  GLuint textureId;
  glGenTextures(1, &textureId);
  if (desc.cubemap) {

    bindTexture(textureId, GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  } else {

    bindTexture(textureId, GL_TEXTURE_2D);
    if (desc.clamp) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    if (!desc.useNeareast) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_NEAREST_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
  }
  return textureId;
}

GLuint OpenGLDriver::createRenderbuffer(RenderBufferDesc desc) {

  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, desc.format, desc.width, desc.height);
  SANITY(glBindRenderbuffer(GL_RENDERBUFFER, 0);)
  return rbo;
}
GLuint OpenGLDriver::createFramebuffer(FrameBufferDesc desc) {
  GLuint frameBuffer;
  glCreateFramebuffers(1, &frameBuffer);
  bindFrameBuffer(frameBuffer);

  for (int i = 0; i < desc.attachmentCount; i++) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, i + GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, desc.attachments[i], 0);
  }

  if (desc.depthAttachment != -1) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           desc.depthAttachment, 0);
  }
  if (desc.stencilAttachment != -1) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           desc.stencilAttachment, 0);
  }

  if (desc.renderBufferAttachment != -1) {
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, desc.renderBufferAttachment);
  }

  if (desc.attachmentCount == 0) {
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  } else {
    const static unsigned int buffers[16] = {
        GL_COLOR_ATTACHMENT0,  GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4,  GL_COLOR_ATTACHMENT5,  GL_COLOR_ATTACHMENT6,
        GL_COLOR_ATTACHMENT7,  GL_COLOR_ATTACHMENT8,  GL_COLOR_ATTACHMENT9,
        GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11, GL_COLOR_ATTACHMENT12,
        GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15,
    };

    glDrawBuffers(desc.attachmentCount, buffers);
  }
  bindFrameBuffer(0);
  return frameBuffer;
}

void OpenGLDriver::bindFrameBuffer(GLuint frameBuffer) {
  if (!bindState.isFrameBufferBound(frameBuffer)) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    bindState.bindFrameBuffer(frameBuffer);
  }
}

void OpenGLDriver::uploadTexture(TextureUploadDesc desc) {

  bindTexture(desc.textureID, desc.target);
  if (desc.isDepth) {
    glTexImage2D(desc.target, 0, GL_DEPTH_COMPONENT, desc.width, desc.height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }

  else if (desc.isDepthStencil) {
    glTexImage2D(desc.target, 0, GL_DEPTH24_STENCIL8, desc.width, desc.height,
                 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
  } else if (desc.isStencil) {
    glTexImage2D(desc.target, 0, GL_DEPTH_COMPONENT, desc.width, desc.height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  } else {

    // TODO internal format hdr
    glTexImage2D(desc.target, 0, desc.format.internalFormat, desc.width,
                 desc.height, 0, desc.format.externalFormat, desc.format.type,
                 desc.buffer);
  }
}
GLuint OpenGLDriver::getUniform(GLuint program, const char *name) {
  return glGetUniformLocation(program, name);
}

void OpenGLDriver::disposeShader(GLuint shader) { glDeleteShader(shader); }
void OpenGLDriver::disposeProgram(GLuint program) { glDeleteProgram(program); }
void OpenGLDriver::disposeTexture(GLuint texture) {
  glDeleteTextures(1, &texture);
}
void OpenGLDriver::disposeVao(GLuint vao) { glDeleteVertexArrays(1, &vao); }
void OpenGLDriver::disposeBuffer(GLuint vbo) { glDeleteBuffers(1, &vbo); }

void OpenGLDriver::bindAttribute(AttributeDesc desc) {
  if (!bindState.isAttributeBound(desc.index)) {
    glEnableVertexAttribArray(desc.index);
    glVertexAttribPointer(desc.index, desc.size, GL_FLOAT, GL_FALSE,
                          desc.stride, (void *)(size_t(desc.offset)));

    if (desc.divisor) {
      glVertexAttribDivisor(desc.index, desc.divisor);
    }

    bindState.bindAttribute(desc.index);
  }
}

void OpenGLDriver::bindProgram(GLuint program) {
  if (!bindState.isProgramBound(program)) {
    glUseProgram(program);
    bindState.bindProgram(program);
  }
}
void OpenGLDriver::bindVao(GLuint vao) {
  if (!bindState.isVAOBound(vao)) {
    glBindVertexArray(vao);
    bindState.bindVao(vao);
  }
}
void OpenGLDriver::bindBuffer(GLuint vbo) {

  if (!bindState.isBufferBound(vbo)) {
    glBindBuffer(bindState.getBufferInformation(vbo).type, vbo);
    bindState.bindBuffer(vbo);
  }
}

// TODO: Use regular glUniform
void OpenGLDriver::bindUniform(GLuint program, GLuint id, GLenum type,
                               void *value, int count) {
  switch (type) {
  case SH_UNIFORM_FLOAT:
    glProgramUniform1fv(program, id, count, (float *)value);
    break;
  case SH_UNIFORM_VEC2:
    glProgramUniform2fv(program, id, count, (float *)value);
    break;
  case SH_UNIFORM_VEC3:
    glProgramUniform3fv(program, id, count, (float *)value);
    break;
  case SH_UNIFORM_VEC4:
    glProgramUniform4fv(program, id, count, (float *)value);
    break;
  case SH_UNIFORM_INT:
    glProgramUniform1iv(program, id, count, (int *)value);
    break;
  case SH_UNIFORM_MAT2:
    glProgramUniformMatrix2fv(program, id, count, false, (float *)value);
    break;
  case SH_UNIFORM_MAT3:
    glProgramUniformMatrix3fv(program, id, count, false, (float *)value);
    break;
  case SH_UNIFORM_MAT4:
    glProgramUniformMatrix4fv(program, id, count, false, (float *)value);
    break;
  case SH_UNIFORM_SAMPLER:
    static std::vector<int> textureUnits;
    textureUnits.resize(count);
    for (int i = 0; i < count; i++) {
      textureUnits[i] = bindTextureUnit(((GLuint *)value)[i], GL_TEXTURE_2D);
    }
    glProgramUniform1iv(program, id, count, &textureUnits[0]);
    break;
  }
}

VideoDeviceParameters OpenGLDriver::queryDeviceParameters() {}

void OpenGLDriver::setViewport(int width, int height) {
  glViewport(0, 0, width, height);
}
void OpenGLDriver::drawCall(DrawCallArgs args) {

  // set(SH_CULL_FACE_MODE_BACK, true);
  int vertexCount = args.vertexCount;

  if (args.indexed) {
    if (args.instanceCount != 0) {
      glDrawElementsInstanced(args.drawMode, vertexCount, Standard::meshIndexGL,
                              (void *)0, args.instanceCount);
    } else
      glDrawElements(args.drawMode, vertexCount, Standard::meshIndexGL,
                     (void *)0);
  } else {

    if (args.instanceCount != 0) {
      glDrawArraysInstanced(args.drawMode, 0, args.vertexCount,
                            args.instanceCount);
    } else
      glDrawArrays(args.drawMode, 0, args.vertexCount);
  }
}

void OpenGLDriver::clear(GLenum flags) { glClear(flags); }

void OpenGLDriver::set(GLenum property, ConfigurationValue value) {
  if (property == SH_CULL_FACE_MODE_BACK) {
    glCullFace(value.val ? GL_BACK : GL_FRONT);
  } else if (property == SH_CLEAR_COLOR) {
    glClearColor(value.vec4.x, value.vec4.y, value.vec4.z, value.vec4.w);
  } else if (property == GL_SRC_ALPHA || property == GL_DST_ALPHA) {
    glBlendFunc(property, value.val);
  } else {
    if (value.val)
      glEnable(property);
    else
      glDisable(property);
  }
}
