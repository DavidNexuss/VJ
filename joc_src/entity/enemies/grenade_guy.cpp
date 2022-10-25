#include "grenade_guy.hpp"
#include "ext/math.hpp"
#include "ext/util.hpp"
#include "imgui.h"
#include "shambhala.hpp"
#include <ext.hpp>
using namespace shambhala;

GrenadeGuy::GrenadeGuy(ShotComponent *shot) {

  this->shot = shot;
  this->shot->addEntity(this);
  /*
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
  setEntityComponent(this); */
}

void GrenadeGuy::createEnemyClass(int id, EnemyClass enemyClass) {
  enemyClasses[id] = enemyClass;
}

int GrenadeGuy::spawnEnemy(int id, float x, float y) {
  EnemyClass cl = enemyClasses[id];
  EnemyInstance instance;
  instance.model = cl.atlas->createDynamicPart(cl.regularPart);
  instance.model->zIndex = 2;
  shambhala::addModel(instance.model);

  instance.center = shambhala::createNode();
  instance.model->getNode()->setParentNode(instance.center);
  instance.model->getNode()->transform(cl.scaleTransform);
  instance.setImmediateNode(instance.model->getNode());
  instance.setPositionNode(instance.center);
  instance.setDamping(0.8);
  instance.setEntityComponent(this);
  *instance.immediateGetPosition() = glm::vec2(x, y);
  instance.updateNodePosition(*instance.immediateGetPosition());
  instance.enemyClass = id;
  instance.health = cl.maxHealth;
  enemies.push(instance);
  return enemies.size() - 1;
}

int GrenadeGuy::castPoint(glm::vec2 position) {
  for (int i = 0; i < enemies.size(); i++) {
    EnemyInstance instance = enemies[i];
    if (instance.containingBox(instance.getLocalBox()).inside(position)) {
      return i;
    }
  }
  return -1;
}

Collision GrenadeGuy::inside(glm::vec2 position) {
  Collision col;
  if ((col.typeInstance = castPoint(position)) != -1) {
    col.typeClass = COLLISION_ENEMY;
    return col;
  }
  return Collision{};
}

void GrenadeGuy::killEnemy(int index) {
  EnemyInstance instance = enemies[index];
  shambhala::removeModel(instance.model);
  shambhala::destroyModel(instance.model);
  enemies.removeNShift(index);
}
void GrenadeGuy::signalHit(Collision col) {

  int enemy = col.typeInstance;
  EnemyInstance &instance = enemies[col.typeInstance];
  instance.hit = 2.0f;
  glm::vec2 current = instance.getVelocity();
  instance.setVelocity(current +
                       col.velocity / enemyClasses[instance.enemyClass].mass);

  instance.health -= col.damage;
  if (instance.health <= 0.0f) {
    killEnemy(enemy);
  }
}
static float randf() {
  float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  return r * 2.0 - 1.0;
}

void GrenadeGuy::step(shambhala::StepInfo info) {

  for (int i = 0; i < enemies.size(); i++) {
    EnemyInstance &instance = enemies[i];
    EnemyClass &cl = enemyClasses[instance.enemyClass];
    instance.updatePosition(cl.getAcceleration());

    // Update enemy
    {
      instance.animationDelay += viewport()->deltaTime;
      if (instance.isShooting) {
        instance.shootDelay += viewport()->deltaTime;
        // Shoot
        if (instance.shootDelay > cl.shootThreshold) {

          instance.shootDelay = 0.0f;
          for (int i = 0; i < cl.shootCount; i++) {
            glm::vec2 shotPosition = cl.getAbsoluteShotCenter(instance);
            glm::vec2 shotDir = cl.shotDirection;
            shotDir += glm::vec2(randf(), randf()) * cl.shootSpread;
            shot->addShot(shotPosition, shotDir, 1, cl.shotSize,
                          cl.shotGravity);
          }
        }

        // Update anim
        if (instance.animationDelay > cl.shootingAnimationThreshold) {
          instance.animationIndex++;
          instance.animationIndex =
              instance.animationIndex % cl.shootAnimationCount;
          cl.atlas->configureMaterial(instance.animationIndex +
                                          cl.regularAnimationCount,
                                      instance.model->material);
          instance.animationDelay = 0.0f;
        }
      } else {
        // Update animatin
        if (instance.animationDelay > cl.regularAnimationThreshold) {

          instance.animationIndex++;
          instance.animationIndex =
              instance.animationIndex % cl.regularAnimationCount;
          cl.atlas->configureMaterial(instance.animationIndex,
                                      instance.model->material);
          instance.animationDelay = 0.0f;
        }
      }

      // Update uniforms
      {
        instance.hit -= viewport()->deltaTime;
        instance.hit = glm::max(0.0f, instance.hit);
        instance.model->material->set("hit", instance.hit);
      }
    }
  }
}

void GrenadeGuy::editorRender() {
  if (ImGui::Begin("Enemies")) {
    for (int i = 0; i < enemies.size(); i++) {
      EnemyInstance &instance = enemies[i];
      ImGui::InputFloat2(("Position " + std::to_string(i)).c_str(),
                         (float *)instance.immediateGetPosition());
      ImGui::End();
      instance.updateNodePosition(*instance.immediateGetPosition());
    }
  }
}

void GrenadeGuy::editorStep(shambhala::StepInfo info) {

  glm::vec2 pos = ext::rayIntersection(info.mouseRay, ext::zplane(0.0));
  if (viewport()->isRightMousePressed()) {
    spawnEnemy(0, pos.x, pos.y);
  }
}
