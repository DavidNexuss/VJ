#pragma once
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

  virtual void setActiveWindow(void *window) = 0;
  virtual bool isKeyPressed(int keyCode) = 0;
  virtual bool isKeyJustPressed(int keyCode) = 0;
  virtual void hideMouse(bool hide) = 0;
  virtual void fakeViewportSize(int width, int height) = 0;
  virtual void restoreViewport() = 0;
  virtual bool isMousePressed() = 0;

  virtual void *createWindow(const WindowConfiguration &configuration) = 0;
  virtual void dispatchRenderEvents() = 0;
  virtual bool shouldClose() = 0;
};
} // namespace shambhala
