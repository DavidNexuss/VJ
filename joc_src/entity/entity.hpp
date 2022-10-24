#pragma once
#include <core/core.hpp>
#include <shambhala.hpp>

struct DynamicPart {
  glm::vec2 offset;
  glm::vec2 scale;
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

const static int COLLISION_WORLD = 1;
const static int COLLISION_ENEMY = 2;
const static int COLLISION_PLAYER = 3;

struct Collision {
  int typeClass = 0;
  int damage = 0;

  inline bool isEmpty() { return typeClass == 0; }
  inline operator int() { return typeClass != 0; }
};

struct Entity {

  virtual Collision inside(glm::vec2 position) = 0;
  virtual Collision inside(glm::vec2 lower, glm::vec2 higher) = 0;
  virtual void signalHit() = 0;
};

struct EntityComponent {
  simple_vector<Entity *> entities;
  inline void addEntity(Entity *ent) { entities.push(ent); }
};

struct AABB {
  glm::vec2 lower = glm::vec2(0.0);
  glm::vec2 higher = glm::vec2(1.0);

  glm::vec2 corner(int index);
};

struct PhsyicalComponent : public EntityComponent {
  Collision collisionCheck();
  void updatePosition(glm::vec2 acceleration);

  AABB containingBox(AABB originalBox);

  inline virtual AABB getLocalBox() { return AABB{}; }
  inline virtual void handleCollision(Collision col) {
    velocity = glm::vec2(0.0);
  }

  inline void setPositionNode(shambhala::Node *node) { positionNode = node; }
  inline void setImmediateNode(shambhala::Node *node) { immediateNode = node; }

  inline glm::vec2 getVelocity() { return velocity; }
  inline void setVelocity(glm::vec2 v) { velocity = v; }

  inline glm::vec2 *immediateGetPosition() { return &position; }

  inline void setDamping(float f) { damping = f; }

  void updateNodePosition(glm::vec2 position);

private:
  glm::vec2 position = glm::vec2(0.0);
  glm::vec2 velocity = glm::vec2(0.0);
  float damping = 1.0;
  shambhala::Node *immediateNode = nullptr;
  shambhala::Node *positionNode = nullptr;
};
