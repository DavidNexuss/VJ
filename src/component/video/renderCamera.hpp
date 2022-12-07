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
  RenderCameraOuptut *getOutputTexture(int attachmentIndex);

private:
  void configure(RenderShot &);
  int lastFrame;
  int renderSlot;
};
