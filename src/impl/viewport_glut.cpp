#include "viewport_glut.hpp"
#include <GL/freeglut_std.h>
#include <GL/glut.h>

using namespace shambhala;

static void keyboardDownCallback(unsigned char key, int x, int y) {}
static void keyboardUpCallback(unsigned char key, int x, int y) {}
static void motionCallback(int x, int y) {}
static void mouseCallback(int button, int state, int x, int y) {}
static void drawCallback() { glutSwapBuffers(); }
static void idleCallback() {}

void viewportGLUT::setActiveWindow(void *window) {}
void viewportGLUT::hideMouse(bool hide) {}

void viewportGLUT::dispatchRenderEvents() {}
bool viewportGLUT::shouldClose() { return false; }

void *viewportGLUT::createWindow(const WindowConfiguration &configuration) {
  char *appname = (char *)("main");
  int argc = 1;
  glutInit(&argc, &appname);

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(configuration.width, configuration.height);

  glutCreateWindow(configuration.titlename);
  glutDisplayFunc(drawCallback);
  glutIdleFunc(idleCallback);
  glutKeyboardFunc(keyboardDownCallback);
  glutKeyboardUpFunc(keyboardUpCallback);
  glutMouseFunc(mouseCallback);
  glutMotionFunc(motionCallback);
  return nullptr;
}

void viewportGLUT::imguiBeginRender() {}
void viewportGLUT::imguiEndRender() {}
void viewportGLUT::imguiDispose() {}
void viewportGLUT::imguiInit(int openglmajor, int openglminor) {}
