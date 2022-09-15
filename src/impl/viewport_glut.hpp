#include <adapters/viewport.hpp>

namespace shambhala {

struct viewportGLUT : public IViewport {
  virtual void setActiveWindow(void *window) override;
  virtual void hideMouse(bool hide) override;

  virtual void *createWindow(const WindowConfiguration &configuration) override;
  virtual void dispatchRenderEvents() override;
  virtual bool shouldClose() override;
};
} // namespace shambhala
