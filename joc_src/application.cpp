#include "application.hpp"
#include "device/shambhala_audio.hpp"
#include "ext/worldmat.hpp"
#include <ext.hpp>
#include <impl/audio_openal.hpp>
#include <impl/io_linux.hpp>
#include <impl/io_std.hpp>
#include <impl/logger.hpp>
#include <impl/serialize.hpp>
#include <impl/viewport_glfw.hpp>
#include <shambhala.hpp>

using namespace shambhala;

void Joc::enginecreate() {

  // Setup engine and window
  EngineParameters parameters;
  parameters.io = new shambhala::LinuxIO;
  parameters.viewport = new shambhala::ViewportGLFW;
  parameters.logger = new shambhala::DefaultLogger;
  parameters.serializer = new shambhala::StreamSerializer;
  parameters.audio = new AudioOpenAL;

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

  // Adds debug camera
  {
    worldmats::DebugCamera *debugCamera = new worldmats::DebugCamera;
    shambhala::setWorldMaterial(Standard::wCamera, debugCamera);
    addComponent(debugCamera);
  }
  shambhala::setWorkingModelList(mainShot.scenes[0]);
}

void Joc::loop() {
  editor::editorInit();
  do {

    shambhala::audio::loop_stepAudio();
    shambhala::loop_begin();
    {
      shambhala::loop_io_sync_step();
      shambhala::loop_beginRenderContext(mainShot.currentFrame);
      shambhala::loop_componentUpdate();
      mainCamera->render(mainShot);
      shambhala::loop_beginUIContext();
      editor::editorBeginContext();
      editor::editorRender(mainShot.currentFrame);
      editor::editorEndContext();
      shambhala::loop_endUIContext();
      shambhala::loop_endRenderContext();
    }
    shambhala::loop_end();
    mainShot.updateFrame();

  } while (!shambhala::loop_shouldClose());
}
