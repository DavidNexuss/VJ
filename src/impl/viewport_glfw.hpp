#pragma once
#include <unordered_map>
#include <viewport.hpp>

namespace shambhala {
struct ViewportGLFW : public IViewport {
  ViewportGLFW();
  virtual void initCallbacks(void *window) override;
  virtual bool isKeyPressed(int keyCode) override;
  virtual bool isKeyJustPressed(int keyCode) override;
  virtual void hideMouse(bool hide) override;
  virtual void fakeViewportSize(int width, int height) override;
  virtual void restoreViewport() override;
  virtual bool isMousePressed() override;

  bool mousePressed;

  std::unordered_map<int, bool> pressed;
  std::unordered_map<int, bool> justPressed;

  void *createWindow(const WindowConfiguration &configuration) override;
};
} // namespace shambhala
