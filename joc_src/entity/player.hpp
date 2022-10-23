#include "entity.hpp"
#include "shot.hpp"
#include <shambhala.hpp>

struct Player : public shambhala::LogicComponent, EntityComponent {

  Player(ShotComponent *shotComponent, DynamicPartAtlas *atlas);
  void step(shambhala::StepInfo info) override;

  glm::vec2 getShootingCenter();
  AABB containingBox();

  void editorRender() override;

private:
  void updatePlayerPosition();

  float shootingDelay = 0.0;
  float shootingCharge = 0.0;
  shambhala::Node *playerPositionNode;
  glm::vec2 playerPosition = glm::vec2(0.0);
  glm::vec2 playerVelocity = glm::vec2(0.0);

  ShotComponent *shot = nullptr;
  shambhala::Model *ship_model = nullptr;

  bool isPositionValid(shambhala::Node *node);
};
