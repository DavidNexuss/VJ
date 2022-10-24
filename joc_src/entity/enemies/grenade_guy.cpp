#include "grenade_guy.hpp"
#include "ext/util.hpp"
#include "imgui.h"
#include "shambhala.hpp"
#include <ext.hpp>
using namespace shambhala;

static int coords[] = {0,   32, 36, 32, 72,  32, 36, 32, 108, 32, 36, 32,
                       144, 32, 36, 32, 180, 32, 36, 32, 216, 32, 36, 32,

                       0,   64, 36, 32, 72,  64, 36, 32, 108, 64, 36, 32,
                       144, 64, 36, 32, 180, 64, 36, 32};

static int animationCount = 6;
static int shootAnimation = 4;
static float animationThreshold = 0.3;
static float shootingAnimationThreshold = 0.8;
static float shootThreshold = 15.0f;

GrenadeGuy::GrenadeGuy(ShotComponent *shot) {

  this->shot = shot;
  this->shot->addEntity(this);

  shambhala::Program *program =
      loader::loadProgram("programs/dynamic_tiled.fs", "programs/regular.vs");

  Texture *texture = loader::loadTexture("textures/grenade_guy.png", 4);
  texture->useNeareast = true;

  atlas = DynamicPartAtlas::create(program, texture, coords);
  guy_model = atlas->createDynamicPart(0);
  guy_model->zIndex = 2;
  shambhala::addModel(guy_model);

  guy_center = shambhala::createNode();
  guy_model->getNode()->setParentNode(guy_center);
  guy_model->getNode()->transform(util::scale(3.0));

  setImmediateNode(guy_model->getNode());
  setPositionNode(guy_center);
  setDamping(0.8);
  setEntityComponent(this);
}

glm::vec2 GrenadeGuy::getShootCenter() { return glm::vec2(0.2, 0.9); }
void GrenadeGuy::handleCollision(Collision col) {
  if (col.typeClass == COLLISION_PLAYER_SHOT) {
    printf("AUCH!\n");
    hit = true;
  }
  PhsyicalObject::handleCollision(col);
}

Collision GrenadeGuy::inside(glm::vec2 position) {

  Collision col;
  col.typeClass = COLLISION_ENEMY;
  if (containingBox(getLocalBox()).inside(position))
    return col;
  return Collision{};
}

void GrenadeGuy::signalHit(Collision col) {
  PhsyicalObject::handleCollision(col);
}

void GrenadeGuy::step(shambhala::StepInfo info) {
  glm::vec2 acc = glm::vec2(0.0, -5.0);
  if (hit) {
    acc += glm::vec2(100.0);
    hit = false;
  }
  PhsyicalObject::updatePosition(acc);

  animationDelay += viewport()->deltaTime;

  float animThreshold =
      isShooting ? shootingAnimationThreshold : animationThreshold;
  int animCount = isShooting ? shootAnimation : animationCount;
  int baseCount = isShooting ? animationCount : 0;

  if (animationDelay > animationThreshold) {
    animationIndex++;
    animationIndex = animationIndex % animCount;
    atlas->configureMaterial(animationIndex + baseCount, guy_model->material);
    animationDelay = 0.0f;
  }

  shootDelay += viewport()->deltaTime;
  if (shootDelay > shootThreshold && isShooting) {
    shootDelay = 0.0f;
    glm::vec2 position = guy_model->getNode()->getCombinedMatrix() *
                         glm::vec4(getShootCenter(), 0.0, 1.0);
    shot->addShot(position, glm::vec2(-0.5, 0.5) * 3.0f, 1, 3.0f,
                  glm::vec2(0.0, -0.3));
  }
}

void GrenadeGuy::editorRender() {
  if (ImGui::Begin("GrenadeGuy")) {
    ImGui::InputFloat2("Grenade Position", (float *)immediateGetPosition());
    ImGui::End();
    updateNodePosition(*immediateGetPosition());
  }
}
