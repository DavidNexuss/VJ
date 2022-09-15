#define GLFW_STATIC
#include "viewport_glfw.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <unordered_map>

using namespace shambhala;
using Window = GLFWwindow;

static ViewportGLFW *currentViewport;
static Window *currentWindow;

ViewportGLFW::ViewportGLFW() {
  screenWidth = 800;
  screenHeight = 600;
  deltaTime = 0.1;
  mousePressed = false;
}

void cursor_position_callback(Window *window, double x, double y) {
  /* if (GUI::isMouseOnGUI())
    return; */

  currentViewport->xpos = x;
  currentViewport->ypos = y;
}

void framebuffer_size_callback(Window *window, int width, int height) {
  currentViewport->screenWidth = width;
  currentViewport->screenHeight = height;
  glViewport(0, 0, width, height);
}

void scroll_callback(Window *window, double xoffset, double yoffset) {
  currentViewport->scrollX = xoffset;
  currentViewport->scrollY = yoffset;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  bool pressed = action == GLFW_PRESS || action == GLFW_REPEAT;
  currentViewport->justPressed[key] = !currentViewport->pressed[key] && pressed;
  currentViewport->pressed[key] = pressed;
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    currentViewport->mousePressed = true;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    currentViewport->mousePressed = false;
}

void ViewportGLFW::setActiveWindow(void *udata) {
  GLFWwindow *window = (GLFWwindow *)udata;
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  currentViewport = this;
  currentWindow = window;
}

void ViewportGLFW::hideMouse(bool hide) {
  glfwSetInputMode(currentWindow, GLFW_CURSOR,
                   hide ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void ViewportGLFW::dispatchRenderEvents() {
  glfwSwapBuffers(currentWindow);
  glfwPollEvents();
}

bool ViewportGLFW::shouldClose() {
  return glfwWindowShouldClose(currentWindow);
}

using namespace std;
void *ViewportGLFW::createWindow(const WindowConfiguration &configuration) {

  if (!glfwInit()) {
    cerr << "[ERROR] Could not initialize GLFW" << endl;
    return nullptr;
  }

  glfwWindowHint(GLFW_SAMPLES, configuration.mssaLevel);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, configuration.openglMajorVersion);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, configuration.openglMinorVersion);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  Window *window;
  window = glfwCreateWindow(configuration.width, configuration.height,
                            configuration.titlename, NULL, NULL);
  if (window == nullptr) {
    cerr << "[ERROR] Could not create window" << endl;
    cerr << "Creating window with: " << configuration.titlename << endl;
    cerr << "Width: " << configuration.width << endl;
    cerr << "Height: " << configuration.height << endl;
    cerr << "OpenGL: " << configuration.openglMajorVersion << endl;
    cerr << "OpenGL Minor Version: " << configuration.openglMinorVersion
         << endl;
    cerr << "MSSA Level: " << configuration.mssaLevel << endl;

    return nullptr;
  }

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    cerr << "[ERROR] Could not initialize glew" << endl;
    return nullptr;
  }

  return window;
}
