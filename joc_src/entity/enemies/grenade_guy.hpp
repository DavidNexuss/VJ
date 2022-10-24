#include "../entity.hpp"
#include "../shot.hpp"
#include "shambhala.hpp"
#include <glm/common.hpp>

struct EnemyClass {
  DynamicPartAtlas *atlas;
  glm::vec2 shotCenter;
  glm::mat4 scaleTransform;
  int regularPart = 0;
  int attackPart = 0;
};

struct EnemyInstance {
  bool hit = false;
  bool isShooting = false;
  float animationDelay = 0.0;
  float shootDelay = 0.0;
  int animationIndex;
  shambhala::Model *model;
  shambhala::Node *center;
};

struct GrenadeGuy : public PhsyicalObject,
                    public shambhala::LogicComponent,
                    public EntityComponent,
                    public Entity {

  GrenadeGuy(ShotComponent *shot);
  void handleCollision(Collision col) override;

  void step(shambhala::StepInfo info) override;
  void editorRender() override;
  glm::vec2 getShootCenter();

  Collision inside(glm::vec2 position) override;
  void signalHit(Collision col) override;

private:
  bool hit = false;
  bool isShooting = true;
  float animationDelay = 0.0;
  float shootDelay = 0.0;
  DynamicPartAtlas *atlas;
  int animationIndex = 0;
  ShotComponent *shot;
  shambhala::Model *guy_model;
  shambhala::Node *guy_center;
};
