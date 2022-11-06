#include "entity.hpp"
#include "forceShot.hpp"
#include "shot.hpp"
#include <device/shambhala_audio.hpp>
#include <shambhala.hpp>

struct Player : public shambhala::LogicComponent,
                public EntityComponent,
                PhsyicalEntity {

  Player(ShotComponent *shotComponent, ForceShotComponent *force,
         DynamicPartAtlas *atlas);
  void step(shambhala::StepInfo info) override;
  glm::vec2 getShootingCenter();
  void editorRender() override;

  void render() override;
  void handleCollision(Collision col) override;
  Collision inside(glm::vec2 position) override;

  inline shambhala::Node *getPlayerPosition() { return playerPosition; }

  void activateForce(bool);

private:
  void updatePlayerPosition();

  float hit = 0.0;
  float shootingDelay = 0.0;
  float shootingCharge = 0.0;
  bool forceActivated = false;

  ShotComponent *shot = nullptr;
  ForceShotComponent *force = nullptr;

  shambhala::audio::SoundModel *soundModel;
  shambhala::Model *ship_model;
  shambhala::Model *ship_force;
  shambhala::Node *playerPosition;
};
