#pragma once
#include <unordered_map>
namespace shambhala {
struct WindowConfiguration {
  const char *titlename;
  int width;
  int height;
  int openglMajorVersion;
  int openglMinorVersion;
  int mssaLevel;
};
struct IViewport {

  double screenWidth, screenHeight;
  double xpos, ypos;
  double scrollX, scrollY;
  double deltaTime;

  bool isKeyPressed(int keyCode);
  bool isKeyJustPressed(int keyCode);

  void fakeViewportSize(int width, int height);
  void restoreViewport();

  bool isMousePressed();

  virtual void setActiveWindow(void *window) = 0;
  virtual void hideMouse(bool hide) = 0;

  virtual void *createWindow(const WindowConfiguration &configuration) = 0;
  virtual void dispatchRenderEvents() = 0;
  virtual bool shouldClose() = 0;

  std::unordered_map<int, bool> pressed;
  std::unordered_map<int, bool> justPressed;

  bool mousePressed;

private:
  int backedWidth;
  int backedHeight;
};
} // namespace shambhala
