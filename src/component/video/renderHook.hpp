#pragma once
#include "../base.hpp"

struct RenderStepInfo {};
struct RenderHook {
  virtual void renderStep(RenderStepInfo) {}
};
