#pragma once
#include <shambhala.hpp>

struct Joc {
  void enginecreate();
  void loop();

private:
  shambhala::RenderCamera *mainCamera;
  shambhala::RenderShot mainShot;
};
