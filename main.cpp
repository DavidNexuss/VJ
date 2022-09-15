#include <impl/io_linux.hpp>
#include <impl/viewport_glfw.hpp>
#include <iostream>
#include <shambhala.hpp>

using namespace shambhala;

int main() {
  EngineParameters parameters;
  parameters.io = new shambhala::LinuxIO;
  parameters.viewport = new shambhala::ViewportGLFW;
  shambhala::createEngine(parameters);

  WindowConfiguration configuration;
  configuration.titlename = "Test main";
  configuration.width = 800;
  configuration.height = 600;
  configuration.mssaLevel = 2;
  configuration.openglMajorVersion = 4;
  configuration.openglMinorVersion = 3;

  shambhala::setActiveWindow(shambhala::createWindow(configuration));
  do {

    shambhala::loop_beginRenderContext();
    shambhala::loop_declarativeRender();
    shambhala::loop_beginUIContext();
    shambhala::loop_endUIContext();
    shambhala::loop_endRenderContext();

  } while (!shambhala::loop_shouldClose());
}
