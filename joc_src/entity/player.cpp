#include "player.hpp"
#include "ext/util.hpp"
#include "imgui.h"
#include "shambhala.hpp"
#include <glm/ext/vector_common.hpp>

using namespace shambhala;

static float maxShootingCharge = 5.0;
static float minshootingDelay = 1.2;
static float shootspeed = 3.0;
static float playerspeed = 6.0;

Player::Player(ShotComponent *shot, DynamicPartAtlas *atlas) {
  this->shot = shot;
  ship_model = atlas->createDynamicPart(0);
  ship_model->zIndex = 3;
  shambhala::addModel(ship_model);

  playerPosition = shambhala::createNode();
  ship_model->getNode()->setParentNode(playerPosition);
  ship_model->getNode()->transform(util::scale(1.5) *
                                   util::rotate(0.0, 0.0, 1.0, -M_PI * 0.5));

  setImmediateNode(ship_model->getNode());
  setPositionNode(playerPosition);
  setDamping(0.8);
}

glm::vec2 Player::getShootingCenter() {
  return glm::vec2(playerPosition->getCombinedMatrix() *
                   glm::vec4(0.0, 0.0, 0.0, 1.0)) -
         glm::vec2(-0.4, 1.0);
}

void Player::handleCollision(Collision col) {
  if (col.typeClass == 2) {
  }

  PhsyicalComponent::handleCollision(col);
}
void Player::step(shambhala::StepInfo info) {

  // Ship movement
  {
    int x = shambhala::viewport()->isKeyPressed(KEY_L);
    int y = shambhala::viewport()->isKeyPressed(KEY_I);

    x -= viewport()->isKeyPressed(KEY_J);
    y -= viewport()->isKeyPressed(KEY_K);

    glm::vec2 acceleration = glm::vec2(x, y) * playerspeed;
    updatePosition(acceleration);
  }

  // Ship shooting

  {
    this->shootingDelay += viewport()->deltaTime;
    if (shambhala::viewport()->isKeyPressed(KEY_E)) {
      this->shootingCharge += viewport()->deltaTime;
    } else {
      this->shootingCharge = 0.0;
    }

    if (shambhala::viewport()->isKeyPressed(KEY_SPACE)) {
      if (this->shootingDelay >= minshootingDelay) {

        shot->addShot(getShootingCenter(),
                      glm::vec2(shootspeed, 0.0) + getVelocity() * 0.5f, 0,
                      2.0);
        this->shootingDelay = 0.0;
      }
    }
  }
}

void Player::editorRender() {
  if (ImGui::Begin("Player")) {
    ImGui::InputFloat2("Player Position", (float *)immediateGetPosition());
    ImGui::End();
    updateNodePosition(*immediateGetPosition());
  }
}
