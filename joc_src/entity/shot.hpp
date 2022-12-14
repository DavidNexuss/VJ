#pragma once
#include "../atlas.hpp"
#include "entity.hpp"
#include "simple_vector.hpp"
#include <device/shambhala_audio.hpp>
#include <shambhala.hpp>

struct ShotComponent : public shambhala::LogicComponent,
                       public shambhala::Model,
                       public EntityComponent {

  ShotComponent();
  void editorStep(shambhala::StepInfo info) override;
  void editorRender() override;
  void addShot(glm::vec2 position, glm::vec2 direction, int type,
               float sizeoffset = 0.0f,
               glm::vec2 accleration = glm::vec3(0.0, 0.0, 0.0));
  void step(shambhala::StepInfo info) override;
  void draw() override;

private:
  struct Shot {
    int type;
  };

  float startPitch = 1.5;
  float endPitch = 0.7;
  shambhala::audio::SoundMesh *laserStart;
  shambhala::audio::SoundMesh *laserEnd;
  shambhala::Node *soundNode;
  // The shader needs a vector 3
  simple_vector<glm::vec3> shot_acceleration;
  simple_vector<glm::vec3> shot_positions;
  simple_vector<glm::vec3> shot_velocity;
  simple_vector<float> shot_scale;
  simple_vector<Shot> shots;

  void uniformFlush();
  bool needsUniformFlush = false;
};
