#include "player.hpp"
#include "ext/util.hpp"
#include "imgui.h"
#include "shambhala.hpp"
#include <glm/ext/vector_common.hpp>

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
static float playerspeed = 6.0;

static glm::vec2 transformedPosition(glm::vec3 p, Node *node) {
  glm::vec4 result = node->getCombinedMatrix() * glm::vec4(p, 1.0);
  return glm::vec2(result);
}
AABB Player::containingBox() {
  glm::vec2 a = transformedPosition(glm::vec3(0.0), ship_model->getNode());
  glm::vec2 b =
      transformedPosition(glm::vec3(0.0, 1.0, 0.0), ship_model->getNode());
  glm::vec2 c =
      transformedPosition(glm::vec3(1.0, 0.0, 0.0), ship_model->getNode());
  glm::vec2 d =
      transformedPosition(glm::vec3(1.0, 1.0, 0.0), ship_model->getNode());

  glm::vec2 min = glm::min(glm::min(a, b), glm::min(c, d));
  glm::vec2 max = glm::max(glm::max(a, b), glm::max(c, d));
  return AABB{min, max};
}
bool Player::isPositionValid(Node *node) {
  AABB playerAABB = containingBox();
  for (int i = 0; i < entities.size(); i++) {
    if (entities[i]->inside(playerAABB.lower, playerAABB.higher)) {
      return false;
    }
  }
  return true;
}
void Player::updatePlayerPosition() {

  playerPositionNode->setOffset(glm::vec3(playerPosition, 0.15));
}
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

  glm::vec2 acceleration = glm::vec2(x, y);
  playerVelocity += acceleration * playerspeed * viewport()->deltaTime;
  playerVelocity = playerVelocity * 0.8f;
  glm::vec2 oldPlayerPosition = playerPosition;
  playerPosition += playerVelocity * viewport()->deltaTime;
  updatePlayerPosition();
  if (!isPositionValid(playerPositionNode)) {
    playerVelocity = glm::vec2(0.0);
    playerPosition = oldPlayerPosition;
    updatePlayerPosition();
  }
}

void Player::editorRender() {
  if (ImGui::Begin("Player")) {
    ImGui::InputFloat2("Player Position", &playerPosition[0]);
    ImGui::End();

    updatePlayerPosition();
  }
}
