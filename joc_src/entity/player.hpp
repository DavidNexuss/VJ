#pragma once
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

  int health = 50;

private:
  void updatePlayerPosition();

  float maxHealth = 50.0;
  float hit = 0.0;
  float shootingDelay = 0.0;
  float shootingCharge = 0.0;
  bool forceActivated = false;
  bool forceImproved = false;
  bool invulnerable = false;

  ShotComponent *shot = nullptr;
  ForceShotComponent *force = nullptr;

  shambhala::audio::SoundModel *soundModel;
  shambhala::Model *ship_model;
  shambhala::Model *ship_force;
  shambhala::Node *playerPosition;

  shambhala::Texture *hud;
  shambhala::Texture *life_hud;
  shambhala::Texture *life_hud_foreground;
  shambhala::Program *hudProgram;
  shambhala::Material *hudMaterial;
  shambhala::Mesh *hudMesh;

  void renderHealthBar(glm::vec2 position, float size, float health,
                       glm::vec4 color);
};
