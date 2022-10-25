#include "entity.hpp"
#include "shot.hpp"
#include <device/shambhala_audio.hpp>
#include <shambhala.hpp>

struct Player : public shambhala::LogicComponent,
                public EntityComponent,
                PhsyicalEntity {

  Player(ShotComponent *shotComponent, DynamicPartAtlas *atlas);
  void step(shambhala::StepInfo info) override;
  glm::vec2 getShootingCenter();
  void editorRender() override;

  void handleCollision(Collision col) override;
  Collision inside(glm::vec2 position) override;

private:
  void updatePlayerPosition();

  float hit = 0.0;
  float shootingDelay = 0.0;
  float shootingCharge = 0.0;

  ShotComponent *shot = nullptr;

  shambhala::audio::SoundModel *soundModel;
  shambhala::Model *ship_model;
  shambhala::Node *playerPosition;
};
