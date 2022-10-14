#include "application.hpp"
#include <ext.hpp>
#include <impl/io_linux.hpp>
#include <impl/io_std.hpp>
#include <impl/logger.hpp>
#include <impl/viewport_glfw.hpp>
#include <shambhala.hpp>

using namespace shambhala;

void Joc::enginecreate() {

  // Setup engine and window
  EngineParameters parameters;
  parameters.io = new shambhala::LinuxIO;
  parameters.viewport = new shambhala::ViewportGLFW;
  parameters.logger = new shambhala::DefaultLogger;
  parameters.io->translators.push_back("assets/%s");
  parameters.io->translators.push_back("internal_assets/%s");
  parameters.io->translators.push_back("machine/%s");
  parameters.io->translators.push_back("joc2d/%s");
  shambhala::createEngine(parameters);

  WindowConfiguration configuration;
  configuration.titlename = "Test main";
  configuration.width = 800;
  configuration.height = 600;
  configuration.mssaLevel = 4;
  configuration.openglMajorVersion = 4;
  configuration.openglMinorVersion = 3;

  shambhala::setActiveWindow(shambhala::createWindow(configuration));

  // Initialize engine components
  mainCamera = shambhala::createRenderCamera();
  mainShot.scenes.push(shambhala::createModelList());

  shambhala::setWorldMaterial(Standard::wCamera, new worldmats::DebugCamera);
  shambhala::setWorkingModelList(mainShot.scenes[0]);
}

void Joc::loop() {
  do {

    shambhala::loop_beginRenderContext();
    shambhala::loop_componentUpdate();
    mainCamera->render(mainShot);
    shambhala::loop_endRenderContext();
    mainShot.updateFrame();

  } while (!shambhala::loop_shouldClose());
}
