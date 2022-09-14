#pragma once
#include <controllers/viewport.hpp>
#include <unordered_map>

namespace shambhala {
struct ViewportGLFW : public IViewport {
  ViewportGLFW();

  virtual void setActiveWindow(void *window) override;
  virtual bool isKeyPressed(int keyCode) override;
  virtual bool isKeyJustPressed(int keyCode) override;
  virtual void hideMouse(bool hide) override;
  virtual void fakeViewportSize(int width, int height) override;
  virtual void restoreViewport() override;
  virtual bool isMousePressed() override;
  virtual void dispatchRenderEvents() override;
  virtual bool shouldClose() override;

  bool mousePressed;

  std::unordered_map<int, bool> pressed;
  std::unordered_map<int, bool> justPressed;

  void *createWindow(const WindowConfiguration &configuration) override;
};
} // namespace shambhala
