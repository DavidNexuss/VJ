#include "engine.hpp"
#include "adapters/video.h"
#include "adapters/video.hpp"
#include <glm/glm.hpp>

void Window::begin() { viewport::setActiveWindow(this->window); }
void Window::end() { viewport::dispatchRenderEvents(); }
bool Window::shouldClose() { return viewport::shouldClose(); }

Window *Engine::spawnWindow(WindowConfiguration configuration) {
  Window *result = new Window;
  result->window = viewport::createWindow(configuration);
  viewport::setActiveWindow(result->window);
  viewport::imguiInit(configuration.openglMajorVersion,
                      configuration.openglMinorVersion);
  return result;
}

void Engine::beginRenderContext() {
  static int frame = 0;
  viewport::notifyFrame(frame++);

  video::set(GL_BLEND, true);
  video::set(GL_SRC_ALPHA, GL_SRC_ALPHA);
  video::set(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  video::set(video::SH_CLEAR_COLOR, glm::vec4{1.0, 0.0, 0.0, 1.0});
  video::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  video::setViewport(viewport::getViewportWidth(),
                     viewport->getViewportHeight());
  vidStepBegin();
  video::stepBegin();
}

void Engine::endRenderContext() {
  viewport::notifyFrameEnd();
  video::stepEnd();
}

void Engine::beginUIRenderContext() { viewport::imguiBeginRender(); }
void Engine::endUIRenderContext() { viewport::imguiEndRender(); }

void Engine::stepAudio() {}
void Engine::stepComponent() {}

void Engine::stepPollEvents() { viewport::pollEvents(); }

void Engine::renderScene() {}
static Engine *current;
Engine *createEngine(EngineControllers controllers) {
  Engine *result = new Engine;
  current = result;
  *(EngineControllers *)result = controllers;
  return result;
}
void disposeEngine(Engine *) {}
