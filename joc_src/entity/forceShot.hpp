#pragma once
#include "entity.hpp"
#include <shambhala.hpp>

struct ForceShotComponent : public shambhala::LogicComponent, EntityComponent {

  void addShot(glm::vec2 start, glm::vec2 velocity, glm::vec3 tint,
               float duration);

  void step(shambhala::StepInfo info) override;
  void render() override;

  void editorRender() override;
  ForceShotComponent();

private:
  shambhala::Program *forceProgram = nullptr;
  shambhala::Mesh *mesh = nullptr;
  bool debugSpawn = false;

  Collision collide(glm::vec2 p);
  struct ForceShot {
    glm::vec2 position;
    glm::vec2 velocity;
    float lastingDuration;
    shambhala::VertexBuffer *buffer;
  };

  simple_vector<ForceShot> shots;
};
