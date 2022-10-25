#pragma once
#include <adapters/viewport.hpp>
#include <unordered_map>

namespace shambhala {
struct ViewportGLFW : public IViewport {
  ViewportGLFW();

  virtual void setActiveWindow(void *window) override;
  virtual void hideMouse(bool hide) override;
  virtual void dispatchRenderEvents() override;
  virtual bool shouldClose() override;

  void imguiInit(int openglmajor, int openglminor) override;
  void imguiDispose() override;
  void imguiBeginRender() override;
  void imguiEndRender() override;

  void *createWindow(const WindowConfiguration &configuration) override;

private:
  float last_time = 0;
};
} // namespace shambhala
