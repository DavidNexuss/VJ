#include "model.hpp"
#include "renderHook.hpp"

struct RenderTarget {
  std::vector<Model *> *modelList;
  std::vector<RenderHook *> *renders;
};
