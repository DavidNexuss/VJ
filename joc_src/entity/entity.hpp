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

  void configureNode(int part, shambhala::Node *node);
  void configureMaterial(int part, shambhala::Material *material);

private:
  int editor_part = 0;
};

const static int COLLISION_WORLD = 1;
const static int COLLISION_ENEMY = 2;
const static int COLLISION_PLAYER = 3;
const static int COLLISION_ENEMY_SHOT = 4;
const static int COLLISION_PLAYER_SHOT = 5;

struct Collision {
  int typeClass = 0;
  int typeInstance = 0;
  glm::vec2 velocity;
  glm::vec2 shortestPosition;
  int damage = 0;

  inline bool isEmpty() { return typeClass == 0; }
  inline operator int() { return typeClass != 0; }
};

struct AABB {
  glm::vec2 lower = glm::vec2(0.0);
  glm::vec2 higher = glm::vec2(1.0);

  glm::vec2 corner(int index);
  inline bool inside(glm::vec2 point) {
    return point.x <= higher.x && point.x >= lower.x && point.y <= higher.y &&
           point.y >= lower.y;
  }
};

struct Entity {

  virtual Collision inside(glm::vec2 position) = 0;
  virtual inline Collision inside(AABB collisionBox) {
    Collision col;
    col = inside(collisionBox.corner(0));
    if (!col.isEmpty())
      return col;
    col = inside(collisionBox.corner(1));
    if (!col.isEmpty())
      return col;

    col = inside(collisionBox.corner(2));
    if (!col.isEmpty())
      return col;

    col = inside(collisionBox.corner(3));
    if (!col.isEmpty())
      return col;
    return Collision{};
  }

  virtual inline Collision inside(glm::vec2 lower, glm::vec2 higher) {
    return inside(AABB{lower, higher});
  }
  virtual void signalHit(Collision col) = 0;
};

struct EntityComponent {
  simple_vector<Entity *> entities;
  inline void addEntity(Entity *ent) { entities.push(ent); }
};

struct PhsyicalObject {
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
  inline void setPosition(glm::vec2 v) { position = v; }

  inline glm::vec2 *immediateGetPosition() { return &position; }

  inline void setDamping(float f) { damping = f; }

  void updateNodePosition(glm::vec2 position);

  void setEntityComponent(EntityComponent *comp);

private:
  bool updatePositionStep(glm::vec2 acceleration, float delta);
  EntityComponent *component;
  glm::vec2 position = glm::vec2(0.0);
  glm::vec2 velocity = glm::vec2(0.0);
  float damping = 1.0;
  shambhala::Node *immediateNode = nullptr;
  shambhala::Node *positionNode = nullptr;
};

struct PhsyicalEntity : public Entity, public PhsyicalObject {
  void signalHit(Collision col) override { handleCollision(col); }
};
