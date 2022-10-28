#include "application.hpp"
#include "device/shambhala_audio.hpp"
#include "ext/worldmat.hpp"
#include <ext.hpp>
#include <impl/audio_openal.hpp>
#include <impl/io_linux.hpp>
#include <impl/io_std.hpp>
#include <impl/logger.hpp>
#include <impl/serialize.hpp>
#include <shambhala.hpp>

#ifdef GLFW
#include <impl/viewport_glfw.hpp>
#endif

#ifdef GLUT
#include <impl/viewport_glut.hpp>
#endif

using namespace shambhala;

void Joc::enginecreate() {

  // Setup engine and window
  EngineParameters parameters;
  parameters.io = new shambhala::LinuxIO;
#ifdef GLUT
  parameters.viewport = new shambhala::viewportGLUT;
#endif

#ifdef GLFW
  parameters.viewport = new shambhala::ViewportGLFW;
#endif
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
    shambhala::pushMaterial(debugCamera);
    addComponent(debugCamera);
  }
  shambhala::setWorkingModelList(mainShot.scenes[0]);
}

void Joc::loop() {
#ifdef EDITOR
  editor::editorInit();
#endif

  int frame = 0;
  do {

    shambhala::audio::loop_stepAudio();
    shambhala::loop_begin();
    {

      getWorkingModelList()->use();
      shambhala::loop_io_sync_step();
      shambhala::loop_beginRenderContext(frame);
      shambhala::loop_componentUpdate();
      shambhala::device::renderPass();
      // mainCamera->render(mainShot);
#ifdef EDITOR
      shambhala::loop_beginUIContext();
      editor::editorBeginContext();
      editor::editorRender(frame);
      editor::editorEndContext();
      shambhala::loop_endUIContext();
#endif
      shambhala::loop_endRenderContext();
    }
    shambhala::loop_end();
    frame++;

  } while (!shambhala::loop_shouldClose());
}
