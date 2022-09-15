#include <adapters/viewport.hpp>

namespace shambhala {

struct viewportGLUT : public IViewport {
  virtual void setActiveWindow(void *window) override;
  virtual bool isKeyPressed(int keyCode) override;
  virtual bool isKeyJustPressed(int keyCode) override;
  virtual void hideMouse(bool hide) override;
  virtual void fakeViewportSize(int width, int height) override;
  virtual void restoreViewport() override;
  virtual bool isMousePressed() override;

  virtual void *createWindow(const WindowConfiguration &configuration) override;
  virtual void dispatchRenderEvents() override;
  virtual bool shouldClose() override;
};
} // namespace shambhala
