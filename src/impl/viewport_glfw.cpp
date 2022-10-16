#include "shambhala.hpp"
#include <string>
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
  if (!currentViewport->isInputEnabled())
    return;

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
  if (pressed) {
    currentViewport->setKeyPressed(key, true);
  }
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {

  if (button == GLFW_MOUSE_BUTTON_LEFT)
    currentViewport->mousePressed = action == GLFW_PRESS;

  if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    currentViewport->middleMousePressed = action == GLFW_PRESS;

  if (button == GLFW_MOUSE_BUTTON_RIGHT)
    currentViewport->rightMousePressed = action == GLFW_PRESS;
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

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void ViewportGLFW::imguiInit(int openglMajorVersion, int openglMinorVersion) {

  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(currentWindow, true);

  std::string version = "#version " + std::to_string(openglMajorVersion) +
                        std::to_string(openglMinorVersion) + "0 ";
  ImGui_ImplOpenGL3_Init(version.c_str());
}
void ViewportGLFW::imguiDispose() {

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
void ViewportGLFW::imguiBeginRender() {

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}
void ViewportGLFW::imguiEndRender() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
