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

Player::Player(ShotComponent *shot, ForceShotComponent *force,
               DynamicPartAtlas *atlas) {
  this->shot = shot;
  shot->addEntity(this);
  ship_model = atlas->createDynamicPart(0);
  ship_model->zIndex = 3;
  shambhala::addModel(ship_model);

  playerPosition = shambhala::createNode();

  ship_model->getNode()->setParentNode(playerPosition);

  glm::mat4 tr = util::scale(1.5f) * util::rotate(0.0, 0.0, 1.0, -M_PI * 0.5f) *
                 util::translate(-0.5f, -0.5f, 0.0);

  ship_model->getNode()->transform(tr);
  this->force = force;

  setImmediateNode(ship_model->getNode());
  setPositionNode(playerPosition);
  setDamping(0.8);
  setEntityComponent(this);

  // Force
  {
    ship_force = shambhala::createModel();
    ship_force->mesh = util::createTexturedQuad();
    ship_force->program =
        loader::loadProgram("programs/tiled.fs", "programs/tiled.vs");
    ship_force->material = shambhala::createMaterial();
    ship_force->material->set("base",
                              loader::loadTexture("textures/force.png", 4));
    ship_force->node = shambhala::createNode();
    ship_force->node->setParentNode(playerPosition);
    ship_force->node->transform(util::scale(3.5f) *
                                util::translate(-0.5f, -0.55f, 0.0));
    ship_force->material->set("add", glm::vec4(0.0));
    ship_force->material->set("mul", glm::vec4(1.2));
    ship_force->material->set("stScale", glm::vec2(1.0));
    ship_force->material->set("stOffset", glm::vec2(0.0));
    ship_force->zIndex = 3;
    ship_force->getNode()->setEnabled(false);
    addModel(ship_force);
  }
  this->soundModel = shambhala::audio::createSoundModel();
  this->soundModel->node = ship_model->getNode();
  this->soundModel->mesh = shambhala::audio::createSoundMesh();
  this->soundModel->mesh->soundResource.acquire(
      shambhala::audio::createSoundResource("joc2d/music/music.wav"));
  this->soundModel->loop = true;
  this->soundModel->play();

  // HUd
  {
    hud = loader::loadTexture("textures/hud.png", 4);
    life_hud = loader::loadTexture("textures/life_hud.png", 4);
    life_hud_foreground =
        loader::loadTexture("textures/life_hud_foreground.png", 4);

    hud->useNeareast = true;
    life_hud->useNeareast = true;
    life_hud_foreground->useNeareast = true;

    hudMesh = util::createTexturedQuad();
    hudMaterial = shambhala::createMaterial();
    hudMaterial->set("mul", glm::vec4(1.0));
    hudMaterial->set("add", glm::vec4(0.0));
    hudMaterial->set("stOffset", glm::vec2(0.0));
    hudMaterial->set("stScale", glm::vec2(1.0));
    hudProgram = loader::loadProgram("programs/tiled.fs", "programs/tiled.vs");
  }
}

void Player::activateForce(bool p) {
  if (forceActivated) {
    forceImproved = true;
  }
  forceActivated = p;
  forceImproved &= forceActivated;
  ship_force->getNode()->setEnabled(p);

  if (forceImproved) {
    ship_force->material->set("mul", glm::vec4(0.2, 0.4, 1.2, 1.0));
  } else {
    ship_force->material->set("mul", glm::vec4(0.4, 1.2, 0.4, 1.0));
  }
}

glm::vec2 Player::getShootingCenter() {

  return glm::vec2(ship_model->getNode()->getCombinedMatrix() *
                   glm::vec4(0.5, 0.5, 0.0, 1.0));
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

        if (forceActivated) {
          force->addShot(getShootingCenter(),
                         glm::vec2(shootspeed * 2.0, 10.0) +
                             getVelocity() * 0.5f,
                         glm::vec3(20.0), 10.0);

          force->addShot(getShootingCenter(),
                         glm::vec2(shootspeed * 2.0, -10.0) +
                             getVelocity() * 0.5f,
                         glm::vec3(20.0), 10.0);
        }
        this->shootingDelay = 0.0;
      }
    }
  }

  // Uniform update
  {
    hit -= viewport()->deltaTime;
    hit = glm::max(hit, 0.0f);
    ship_model->material->set("hit", hit);
    ship_model->material->set("add", glm::vec4(0.0));
    ship_model->material->set("mul", glm::vec4(1.0));
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

void Player::renderHealthBar(glm::vec2 position, float size, float health) {

  float ra = 8.0;
  glm::mat4 tr = util::translate(position.x, position.y, 0.0) *
                 util::scale(size * ra, size, 1.0) *
                 util::translate(-0.5, -0.5, 0.0);

  hudProgram->bind(Standard::uProjectionMatrix, glm::mat4(1.0));
  hudProgram->bind(Standard::uViewMatrix, glm::mat4(1.0));
  hudProgram->bind(Standard::uTransformMatrix, tr);
  hudProgram->bind("base", hud);

  shambhala::device::drawCall();

  float x = size * ra * health;

  glm::mat4 tr2 = util::translate(position.x, position.y, 0.0) *
                  util::translate(-x * 0.5, -0.5 * size, 0) *
                  util::scale(x, size, 1.0);

  hudProgram->bind(Standard::uTransformMatrix, tr2);
  hudProgram->bind("base", life_hud);

  shambhala::device::drawCall();

  hudProgram->bind(Standard::uProjectionMatrix, glm::mat4(1.0));
  hudProgram->bind(Standard::uViewMatrix, glm::mat4(1.0));
  hudProgram->bind(Standard::uTransformMatrix, tr);
  hudProgram->bind("base", life_hud_foreground);

  shambhala::device::drawCall();
}
void Player::render() {
  hudProgram->use();
  hudProgram->bind(hudMaterial);
  hudMesh->use();

  static float debugTime = 0.0;
  debugTime += viewport()->deltaTime;
  float health = glm::cos(debugTime) * 0.5f + 0.5f;

  renderHealthBar(glm::vec2(0.0, 0.8), 0.1, health);
}
