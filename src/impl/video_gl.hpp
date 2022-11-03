#pragma once
#include "adapters/video.hpp"

namespace shambhala {
namespace video {

struct OpenGLDriver : public video::IVideo {

  /** ENGINE CLEAN UP HOOKS **/

  void initDevice() override;
  void enableDebug(bool) override;

  void stepBegin() override;
  void stepEnd() override;

  /** DRIVER STATUS **/

  ProgramStatus &statusProgramCompilation() override;
  ShaderStatus &statusShaderCompilation() override;

  /** CREATE AND COMPILE FUNCTIONS **/

  // Shaders
  GLuint compileShader(ShaderDesc) override;
  GLuint compileProgram(ProgramDesc) override;

  // Buffers
  GLuint createBuffer(BufferDesc) override;
  GLuint createTexture(TextureDesc) override;
  GLuint createFramebuffer(FrameBufferDesc) override;
  GLuint createRenderbuffer(RenderBufferDesc) override;

  void uploadTexture(TextureUploadDesc) override;
  void uploadBuffer(BufferUploadDesc) override;
  GLuint getUniform(GLuint program, const char *name) override;

  /** DISPOSE FUNCTIONS **/

  void disposeShader(GLuint shader) override;
  void disposeProgram(GLuint program) override;
  void disposeTexture(GLuint texture) override;
  void disposeBuffer(GLuint bo) override;

  /** BIND FUNCTIONS **/

  void bindAttribute(AttributeDesc) override;

  void bindProgram(GLuint program) override;
  void bindVao(GLuint vao) override;
  void bindBuffer(GLuint buffer) override;
  void bindFrameBuffer(GLuint frameBuffer) override;
  void bindUniform(GLuint program, GLuint id, GLenum type, void *value,
                   int count = 1) override;

  /**MISC **/
  VideoDeviceParameters queryDeviceParameters() override;
  void setViewport(int width, int height) override;

  /** DRAW **/
  void clear(GLenum flags) override;
  void drawCall(DrawCallArgs args) override;

  /** DRAW STATE **/

  void set(GLenum property, ConfigurationValue value) override;

private:
  void bindTexture(GLuint textureID, GLenum target);
  int bindTexture(GLuint textureID, GLenum target, GLuint unit);
  int bindTextureUnit(GLuint textureID, GLenum target);
};
} // namespace video

} // namespace shambhala
