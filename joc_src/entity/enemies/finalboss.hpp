#pragma once
#include "../entity.hpp"
#include "../forceShot.hpp"
#include "../shot.hpp"
#include "shambhala.hpp"

struct FinalBoss : public shambhala::Model,
                   public shambhala::LogicComponent,
                   public Entity {

  FinalBoss();

  void step(shambhala::StepInfo info) override;
  void draw() override;

  Collision inside(glm::vec2 position) override;
  void signalHit(Collision col) override;

  float health = 200.0;
  float maxHealth = 200.0;

  bool active = true;

  ShotComponent *shot;
  ForceShotComponent *force;

  glm::vec2 offset;

private:
  float globalStep = 0.0f;
  float shootDelta = 10.0;
  struct Path {
    glm::vec2 position = glm::vec2(0.0);
    glm::vec2 velocity = glm::vec2(0.0);
    float t = 0;
    int index = 0;

    glm::mat4 getTransform();
    glm::mat4 getCollisionTransform();

    static Path interpolate(Path start, Path end, float t);
  };

  struct FinalBossPart {
    float step;
    float health;
    float damage = 0.0;
    float shotdelay = 0.0;
    bool shoot = false;

    glm::vec2 lastPosition;
    glm::mat4 lastMat;
    glm::mat4 lastRotMat;
  };

  struct ST {
    glm::vec2 offset;
    glm::vec2 scale;
  };

  ST getST(Path path);
  Path getPath(float t);

  void attackSequence1(float t);

  shambhala::Node *rootNode;
  shambhala::Texture *texture;
  shambhala::Program *program;
  shambhala::Mesh *mesh;
  simple_vector<FinalBossPart> parts;
};
