#pragma once
#include "adapters/video.hpp"
#include "adapters/viewport.hpp"

struct Engine;
struct Window {
  void begin();
  void end();

  bool shouldClose();

private:
  void *window;
  friend Engine;
};

struct Engine {
  Window *spawnWindow(WindowConfiguration configuration);

  void stepComponent();
  void stepAudio();

  void beginRenderContext();
  void beginUIRenderContext();
  void endUIRenderContext();
  void endRenderContext();

  void stepPollEvents();
  void renderScene();
};

Engine *createEngine();
void disposeEngine(Engine *);

Engine *engine();
