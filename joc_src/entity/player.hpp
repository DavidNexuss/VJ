#include "entity.hpp"
#include "shot.hpp"
#include <shambhala.hpp>

struct Player : public shambhala::LogicComponent, public PhsyicalComponent {

  Player(ShotComponent *shotComponent, DynamicPartAtlas *atlas);
  void step(shambhala::StepInfo info) override;
  glm::vec2 getShootingCenter();
  void editorRender() override;

  void handleCollision(Collision col) override;

private:
  void updatePlayerPosition();

  float shootingDelay = 0.0;
  float shootingCharge = 0.0;

  ShotComponent *shot = nullptr;

  shambhala::Model *ship_model;
  shambhala::Node *playerPosition;
};
