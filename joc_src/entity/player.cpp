#include "player.hpp"
#include "../globals.hpp"
#include "adapters/audio.hpp"
#include "device/shambhala_audio.hpp"
#include "ext/math.hpp"
#include "ext/util.hpp"
#include "imgui.h"
#include "shambhala.hpp"
#include <glm/ext/vector_common.hpp>

using namespace shambhala;

static float maxShootingCharge = 5.0;
static float minshootingDelay = 1.2;
static float shootspeed = 6.0;
static float playerspeed = 6.0;

Player::Player(ShotComponent *shot, DynamicPartAtlas *atlas) {
  this->shot = shot;
  shot->addEntity(this);
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
  setEntityComponent(this);

  this->soundModel = shambhala::audio::createSoundModel();
  this->soundModel->node = ship_model->getNode();
  this->soundModel->mesh = shambhala::audio::createSoundMesh();
  this->soundModel->mesh->soundResource.acquire(
      shambhala::audio::createSoundResource("joc2d/music/music.wav"));
  this->soundModel->loop = true;
  this->soundModel->play();
}

glm::vec2 Player::getShootingCenter() {
  return glm::vec2(playerPosition->getCombinedMatrix() *
                   glm::vec4(0.0, 0.0, 0.0, 1.0)) -
         glm::vec2(-0.4, 1.0);
}

void Player::handleCollision(Collision col) {
  if (col.typeClass == COLLISION_ENEMY_SHOT) {
    hit = 2.0;
  }

  PhsyicalObject::handleCollision(col);
}
Collision Player::inside(glm::vec2 position) {
  Collision col;
  col.typeClass = COLLISION_PLAYER;
  if (containingBox(getLocalBox()).inside(position))
    return col;
  return Collision{};
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
      this->shootingCharge = glm::min(this->shootingCharge, 5.0f);
      float t = glm::cos((this->shootingCharge / 5.0f) * M_PI / 2);

      ship_model->material->set("tint", glm::vec3(1.0 * t + 1.2 * (t - 1),
                                                  1.0 * t + 0.4 * (t - 1),
                                                  1.0 * t + 0.4 * (t - 1)));
    } else {

      if (this->shootingCharge >= 5.0f) {
        shot->addShot(getShootingCenter(),
                      glm::vec2(shootspeed, 0.0) + getVelocity() * 0.5f, 0,
                      8.0);
      }
      ship_model->material->set("tint", glm::vec3(1.0));
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

  // Uniform update
  {
    hit -= viewport()->deltaTime;
    hit = glm::max(hit, 0.0f);
    ship_model->material->set("hit", hit);
  }

  // Audio
  {
    audio::SoundListener listener;
    listener.orientation.x = glm::vec3(1.0, 0.0, 0.0);
    listener.orientation.y = glm::vec3(0.0, 1.0, 0.0);
    listener.position = glm::vec3(*immediateGetPosition(), 0.0);
    listener.velocity = glm::vec3(getVelocity(), 0.0);
    listener.use();
  }
}

void Player::editorRender() {
  if (ImGui::Begin("Player")) {
    ImGui::InputFloat2("Player Position", (float *)immediateGetPosition());
    ImGui::InputFloat("Shoot Charge", &this->shootingCharge);
    ImGui::End();
    updateNodePosition(*immediateGetPosition());
  }
}

void Player::render() {

  glm::vec2 v = *immediateGetPosition();
  joc::font->render("abcdefghijklmnopqrstuvwxyz", v, 2.0);
}
