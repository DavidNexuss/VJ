#include <adapters/log.hpp>
#include <adapters/video.hpp>
#include <unordered_map>
#include <vector>

// Should take into incount supported opengl version
#define O45
#define O41

#define SANITY(x)                                                              \
  { x }

#define EXT_BUFFER_CREATE
#define SANITY_TEXTURE_CACHING(cond) true

using namespace shambhala;
using namespace video;

namespace shambhala {
namespace video {
static struct {

  ProgramStatus programStatus;
  ShaderStatus shaderStatus;

} driverState;

struct BufferInformation {
  GLenum type = 0;
  size_t size = 0;
};

struct TextureInformation {
  GLuint textureUnit = 0;
  bool needsMipmap = false;
  bool usesMipmap = false;
};

struct ProgramInformation {
  std::unordered_map<std::string, GLuint> uniformLocations;
};

static struct {

  GLuint currentVao = -1;
  GLuint currentProgram = -1;
  GLuint currentFramebuffer = -1;
  GLuint currentTexture = -1;
  GLuint currentUnit = -1;

  // Buffers
  std::unordered_map<int, GLuint> boundAttributes;
  std::unordered_map<GLuint, BufferInformation> buffers;
  std::unordered_map<GLenum, GLuint> boundBuffers;

  // Textures
  std::unordered_map<int, GLuint> boundUnits;
  std::unordered_map<GLuint, TextureInformation> textureInformation;

  // Programs
  std::unordered_map<GLuint, ProgramInformation> programsInformation;

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

void enableDebug(bool pEnable) {
  if (pEnable) {
    glDebugMessageCallback(glError, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
  } else {
    glDisable(GL_DEBUG_OUTPUT);
  }
}
void initDevice() { glGenVertexArrays(1, &bindState.currentVao); }
void stepBegin() { glBindVertexArray(bindState.currentVao); }
void stepEnd() {}

ProgramStatus &statusProgramCompilation() { return driverState.programStatus; }

ShaderStatus &statusShaderCompilation() { return driverState.shaderStatus; }

GLuint compileShader(ShaderDesc desc) {

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

GLuint compileProgram(ProgramDesc desc) {

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

  for (int i = 0; i < desc.shaderCount; i++) {
    glDetachShader(program, desc.shaders[i]);
  }

  if (InfoLogLength > 0) {
    pStatus.errorMsg.resize(InfoLogLength + 1);
    glGetProgramInfoLog(program, InfoLogLength, NULL, &pStatus.errorMsg[0]);
  }

  pStatus.errored = status == GL_FALSE;
  if (status == GL_FALSE) {
    glDeleteProgram(program);
  }
  return program;
}

// TODO createBuffers 4.5
GLuint createBuffer(BufferDesc desc) {
  GLuint buff;
  glGenBuffers(1, &buff);
  BufferInformation inf;
  inf.type = desc.type;
  bindState.setBufferInformation(inf, buff);
  return buff;
}

void uploadBuffer(BufferUploadDesc desc) {

  bindBuffer(desc.id);
  if (desc.start == 0 &&
      desc.size > bindState.getBufferInformation(desc.id).size) {
    glBufferData(GL_ARRAY_BUFFER, desc.size, desc.buffer, desc.mode);
    bindState.getBufferInformation(desc.id).size = desc.size;
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, desc.start, desc.size, desc.buffer);
  }
}

void bindTexture(GLuint textureId, GLenum target) {
  if (SANITY_TEXTURE_CACHING(bindState.currentTexture != textureId)) {
    glBindTexture(target, textureId);
    bindState.currentTexture = textureId;
    bindState.boundUnits[bindState.currentUnit] = textureId;
  }
}
int bindTexture(GLuint textureId, GLenum target, GLuint unit) {
  if (SANITY_TEXTURE_CACHING(
          bindState.textureInformation[textureId].needsMipmap)) {
    glGenerateTextureMipmap(textureId);
    bindState.textureInformation[textureId].needsMipmap = false;
  }
  if (SANITY_TEXTURE_CACHING(bindState.boundUnits[unit] != textureId)) {
    bindState.currentUnit = unit;
    glActiveTexture(unit + GL_TEXTURE0);
    bindTexture(textureId, target);
  }
  return unit;
}

int bindTextureUnit(GLuint textureId, GLenum target) {
  return bindTexture(textureId, target, textureId % 1000);
}

static bool needsMipmap(GLenum val) {
  return val == GL_NEAREST_MIPMAP_NEAREST || val == GL_NEAREST_MIPMAP_LINEAR ||
         val == GL_LINEAR_MIPMAP_NEAREST || val == GL_LINEAR_MIPMAP_LINEAR;
}

GLuint createTexture(TextureDesc desc) {

  GLuint textureId;
  glGenTextures(1, &textureId);
  bindTexture(textureId, desc.mode);

  if (desc.clamp) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, desc.magFilter);

  bindState.textureInformation[textureId].usesMipmap =
      needsMipmap(desc.minFilter) || needsMipmap(desc.magFilter);

  return textureId;
}

GLuint createRenderbuffer(RenderBufferDesc desc) {

  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, desc.format, desc.width, desc.height);
  SANITY(glBindRenderbuffer(GL_RENDERBUFFER, 0);)
  return rbo;
}
GLuint createFramebuffer(FrameBufferDesc desc) {
  GLuint frameBuffer = desc.oldFramebuffer;
  if (frameBuffer == -1)
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

void bindFrameBuffer(GLuint frameBuffer) {
  if (!bindState.isFrameBufferBound(frameBuffer)) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    bindState.bindFrameBuffer(frameBuffer);
  }
}

void uploadTexture(TextureUploadDesc desc) {

  if (bindState.textureInformation[desc.textureID].usesMipmap) {
    bindState.textureInformation[desc.textureID].needsMipmap = true;
  }
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

    glTexImage2D(desc.target, 0, desc.format.internalFormat, desc.width,
                 desc.height, 0, desc.format.externalFormat, desc.format.type,
                 desc.buffer);
  }
}
GLuint getUniform(GLuint program, const char *name) {

  auto it = bindState.programsInformation[program].uniformLocations.find(
      std::string(name));
  if (it != bindState.programsInformation[program].uniformLocations.end()) {
    return it->second;
  }

  return bindState.programsInformation[program]
             .uniformLocations[std::string(name)] =
             glGetUniformLocation(program, name);
}

void disposeShader(GLuint shader) { glDeleteShader(shader); }
void disposeProgram(GLuint program) {
  bindState.programsInformation.erase(program);
  glDeleteProgram(program);
}
void disposeTexture(GLuint texture) { glDeleteTextures(1, &texture); }
void disposeBuffer(GLuint vbo) { glDeleteBuffers(1, &vbo); }

void bindAttribute(AttributeDesc desc) {
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

void bindProgram(GLuint program) {
  if (!bindState.isProgramBound(program)) {
    glUseProgram(program);
    bindState.bindProgram(program);
  }
}
void bindVao(GLuint vao) {
  if (!bindState.isVAOBound(vao)) {
    glBindVertexArray(vao);
    bindState.bindVao(vao);
  }
}
void bindBuffer(GLuint vbo) {

  if (!bindState.isBufferBound(vbo)) {
    glBindBuffer(bindState.getBufferInformation(vbo).type, vbo);
    bindState.bindBuffer(vbo);
  }
}

void bindUniform(GLuint program, GLuint id, GLenum type, void *value,
                 int count) {

  if (program != bindState.currentProgram) {
    bindProgram(program);
  }
  switch (type) {
  case SH_UNIFORM_FLOAT:
    glUniform1fv(id, count, (float *)value);
    break;
  case SH_UNIFORM_VEC2:
    glUniform2fv(id, count, (float *)value);
    break;
  case SH_UNIFORM_VEC3:
    glUniform3fv(id, count, (float *)value);
    break;
  case SH_UNIFORM_VEC4:
    glUniform4fv(id, count, (float *)value);
    break;
  case SH_UNIFORM_INT:
    glUniform1iv(id, count, (int *)value);
    break;
  case SH_UNIFORM_MAT2:
    glUniformMatrix2fv(id, count, false, (float *)value);
    break;
  case SH_UNIFORM_MAT3:
    glUniformMatrix3fv(id, count, false, (float *)value);
    break;
  case SH_UNIFORM_MAT4:
    glUniformMatrix4fv(id, count, false, (float *)value);
    break;
  case SH_UNIFORM_SAMPLER:
    static std::vector<int> textureUnits;
    textureUnits.resize(count);
    for (int i = 0; i < count; i++) {
      textureUnits[i] = bindTextureUnit(((GLuint *)value)[i], GL_TEXTURE_2D);
    }
    glUniform1iv(id, count, &textureUnits[0]);
    break;
  }
}

VideoDeviceParameters queryDeviceParameters() {
  return VideoDeviceParameters{};
}

void setViewport(int width, int height) {
  if (width < 0 || height < 0)
    return;
  glViewport(0, 0, width, height);
}
void drawCall(DrawCallArgs args) {

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

void clear(GLenum flags) { glClear(flags); }

void set(GLenum property, ConfigurationValue value) {
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
} // namespace video
} // namespace shambhala
