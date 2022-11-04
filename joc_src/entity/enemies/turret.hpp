#include "../entity.hpp"
#include "shambhala.hpp"

struct Turret : public shambhala::LogicComponent {

  Turret(DynamicPartAtlas *atlas);
  void target(shambhala::Node *node);
  void attach(shambhala::Node *attachNode);
  void step(shambhala::StepInfo info) override;
  void editorStep(shambhala::StepInfo info) override;

private:
  float angularVelocity = 0.3f;
  float currentAngle = 0.0f;
  float targetAngle = 0.0f;
  shambhala::Node *target_node = nullptr;
  shambhala::Model *model = nullptr;
  glm::mat4 originalMatrix;
  static shambhala::Node *turret_root;
};
