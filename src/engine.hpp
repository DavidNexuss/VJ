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
  void stepPollEvents();

  void beginRenderContext();
  void beginUIRenderContext();
  void endUIRenderContext();
  void endRenderContext();
};

Engine *createEngine();
void disposeEngine(Engine *);

Engine *engine();
