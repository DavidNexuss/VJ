#include "application.hpp"
#include "device/shambhala_audio.hpp"
#include "ext/util.hpp"
#include "ext/worldmat.hpp"
#include <ext.hpp>
#include <impl/audio_openal.hpp>
#include <impl/logger.hpp>
#include <impl/serialize.hpp>
#include <impl/video_gl.hpp>
#include <shambhala.hpp>

#ifdef GLFW
#include <impl/viewport_glfw.hpp>
#endif

#ifdef GLUT
#include <impl/viewport_glut.hpp>
#endif

#ifdef WIN32
#include <impl/io_std.hpp>
#else
#include <impl/io_linux.hpp>
#endif

using namespace shambhala;

void Joc::enginecreate() {

  // Setup engine and window
  EngineControllers parameters;
#ifdef WIN32
  parameters.io = new shambhala::STDIO;
#else
  parameters.io = new shambhala::LinuxIO;
#endif

#ifdef GLUT
  parameters.viewport = new shambhala::viewportGLUT;
#endif

#ifdef GLFW
  parameters.viewport = new shambhala::ViewportGLFW;
#endif
  parameters.logger = new shambhala::DefaultLogger;
  parameters.serializer = new shambhala::StreamSerializer;
  parameters.audio = new AudioOpenAL;
  parameters.video = new video::OpenGLDriver;

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

  RenderCamera *sourceCamera = shambhala::createRenderCamera();
  // sourceCamera->setWidth(-2);
  // sourceCamera->setHeight(-2);
  sourceCamera->setConfiguration(shambhala::USE_RENDER_BUFFER);
  sourceCamera->addOutput({GL_RGBA16F, GL_RGBA, GL_FLOAT});
  sourceCamera->clearColor = glm::vec4(0.0, 0.0, 0.0, 1.0);

  mainCamera = new PostProcessCamera(

      loader::loadProgram("programs/blend2d.fs", "programs/parallax.vs"));
  mainCamera->set("scene", sourceCamera->renderOutput(0));

  mainShot.scenes.push(shambhala::createModelList());
  // Adds debug camera
#ifdef DEBUGCAMERA
  {
    worldmats::DebugCamera *debugCamera = new worldmats::DebugCamera;
    shambhala::pushMaterial(debugCamera);
    addComponent(debugCamera);
  }
#endif
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
#ifdef EDITOR
      shambhala::loop_io_sync_step();
#endif

      shambhala::loop_beginRenderContext(frame);
      {

        shambhala::loop_componentUpdate();
        mainCamera->render();
        // device::renderPass();

#ifdef EDITOR
        shambhala::loop_beginUIContext();
        editor::editorBeginContext();
        editor::editorRender(frame);
        editor::editorEndContext();
        shambhala::loop_endUIContext();
#endif
      }
      shambhala::loop_endRenderContext();
    }
    shambhala::loop_end();
    frame++;

  } while (!shambhala::loop_shouldClose());
}
