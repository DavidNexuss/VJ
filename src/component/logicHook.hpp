#include "base.hpp"
struct StepInfo {};

struct Logic : public BaseComponent<Logic> {
  virtual void step(StepInfo info) {}
};
