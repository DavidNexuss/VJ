#pragma once
#include <core/core.hpp>
#include <shambhala.hpp>

struct DynamicPart {
  glm::vec2 offset;
  glm::vec2 scale;
};

struct AABB {
  glm::vec2 lower;
  glm::vec2 higher;
};

struct DynamicPartAtlas : public shambhala::LogicComponent {

  void editorRender() override;
  shambhala::Model *createDynamicPart(int part);
  shambhala::Model *createLight();

  shambhala::Program *renderingProgram = nullptr;
  shambhala::Texture *textureAtlas = nullptr;
  int *coords = nullptr;

  static DynamicPartAtlas *create(shambhala::Program *,
                                  shambhala::Texture *text, int *corrds);

private:
  void configureNode(int part, shambhala::Node *node);
  void configureMaterial(int part, shambhala::Material *material);

  int editor_part = 0;
};

struct Entity {

  virtual bool inside(glm::vec2 position) = 0;
  virtual bool inside(glm::vec2 lower, glm::vec2 higher) = 0;
  virtual void signalHit() = 0;
};

struct EntityComponent {
  simple_vector<Entity *> entities;
  inline void addEntity(Entity *ent) { entities.push(ent); }
};
