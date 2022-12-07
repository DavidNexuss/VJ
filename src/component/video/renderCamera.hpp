#include "framebuffer.hpp"
#include "renderTarget.hpp"
#include "texture.hpp"

struct RenderShot {
  std::vector<RenderTarget *> targets;
};

struct RenderCamera;
struct RenderCameraOuptut : public ITexture {
  GLuint gl() override;
  GLenum getMode() override;

private:
  RenderCamera *owner;
  int attachmentIndex;
};

struct RenderCamera : public Material {
  void render(RenderShot &);

  RenderCameraOuptut *renderOutput(int attachmentIndex);
  void addOutput(video::TextureFormat);
  void setConfiguration(FrameBufferDescriptorFlags);

  int getWidth();
  int getHeight();

private:
  void configure(RenderShot &);
  int lastFrame;
  int renderSlot;
};

struct PostProcessCamera : public RenderCamera {

  PostProcessCamera(Program *program);
  PostProcessCamera *create(const char *fragmentShaderName);

private:
  Program *postprocess;
};
