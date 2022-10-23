#include "player.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"

using namespace shambhala;
Player::Player(ShotComponent *shot, DynamicPartAtlas *atlas) {
  this->shot = shot;
  ship_model = atlas->createDynamicPart(0);
  ship_model->zIndex = 3;
  shambhala::addModel(ship_model);

  playerPositionNode = shambhala::createNode();
  ship_model->getNode()->setParentNode(playerPositionNode);
  ship_model->getNode()->transform(util::scale(1.5) *
                                   util::rotate(0.0, 0.0, 1.0, -M_PI * 0.5));
}

glm::vec2 Player::getShootingCenter() {
  return glm::vec2(playerPositionNode->getCombinedMatrix() *
                   glm::vec4(0.0, 0.0, 0.0, 1.0)) -
         glm::vec2(-0.4, 1.0);
}
static float maxShootingCharge = 5.0;
static float minshootingDelay = 1.2;
static float shootspeed = 3.0;
static float playerspeed = 20.0;

void Player::step(shambhala::StepInfo info) {

  this->shootingDelay += viewport()->deltaTime;
  if (shambhala::viewport()->isKeyPressed(KEY_E)) {
    this->shootingCharge += viewport()->deltaTime;
  } else {
    this->shootingCharge = 0.0;
  }

  if (shambhala::viewport()->isKeyPressed(KEY_SPACE)) {
    if (this->shootingDelay >= minshootingDelay) {

      shot->addShot(getShootingCenter(),
                    glm::vec2(shootspeed, 0.0) + playerVelocity * 0.5f, 0, 2.0);
      this->shootingDelay = 0.0;
    }
  }
  int x = shambhala::viewport()->isKeyPressed(KEY_L);
  int y = shambhala::viewport()->isKeyPressed(KEY_I);

  x -= viewport()->isKeyPressed(KEY_J);
  y -= viewport()->isKeyPressed(KEY_K);

  playerVelocity = glm::vec2(x, y) * playerspeed * viewport()->deltaTime;
  playerPosition += playerVelocity * viewport()->deltaTime;

  playerPositionNode->setOffset(glm::vec3(playerPosition, 0.15));
}
