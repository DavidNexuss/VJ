#include <adapters/viewport.hpp>

namespace shambhala {

struct viewportGLUT : public IViewport {
  virtual void setActiveWindow(void *window) override;
  virtual void hideMouse(bool hide) override;

  virtual void *createWindow(const WindowConfiguration &configuration) override;
  virtual void dispatchRenderEvents() override;
  virtual bool shouldClose() override;

  virtual void imguiInit(int openglmajor, int openglminor) override;
  virtual void imguiDispose() override;
  virtual void imguiBeginRender() override;
  virtual void imguiEndRender() override;
};
} // namespace shambhala
