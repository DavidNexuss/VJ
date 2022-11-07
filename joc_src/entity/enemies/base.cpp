#include "base.hpp"
#include "ext/math.hpp"
#include "ext/util.hpp"
#include "imgui.h"
#include "shambhala.hpp"
#include <ext.hpp>
#include <glm/common.hpp>
#include <glm/ext.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>

using namespace shambhala;

static float randf() {
  float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  return r * 2.0 - 1.0;
}
BaseEnemy::BaseEnemy(ShotComponent *shot) {

  this->shot = shot;
  this->shot->addEntity(this);
}

void BaseEnemy::createEnemyClass(int id, EnemyClass enemyClass) {
  enemyClasses[id] = enemyClass;
}

int BaseEnemy::spawnEnemy(int id, float x, float y) {
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
  instance.setDamping(1.0);
  instance.setEntityComponent(this);
  *instance.immediateGetPosition() = glm::vec2(x, y);
  instance.updateNodePosition(*instance.immediateGetPosition());
  instance.enemyClass = id;
  instance.health = cl.maxHealth;
  instance.target = target;
  enemies.push(instance);
  return enemies.size() - 1;
}

int BaseEnemy::castPoint(glm::vec2 position) {
  for (int i = 0; i < enemies.size(); i++) {
    EnemyInstance instance = enemies[i];
    if (instance.containingBox(instance.getLocalBox()).inside(position)) {
      return i;
    }
  }
  return -1;
}

Collision BaseEnemy::inside(glm::vec2 position) {
  Collision col;
  if ((col.typeInstance = castPoint(position)) != -1) {
    col.typeClass = COLLISION_ENEMY;
    col.velocity = enemies[col.typeInstance].getVelocity();
    return col;
  }
  return Collision{};
}

void BaseEnemy::killEnemy(int index) {
  EnemyInstance instance = enemies[index];
  shambhala::removeModel(instance.model);
  shambhala::destroyModel(instance.model);
  enemies.removeNShift(index);
}

void BaseEnemy::signalHit(Collision col) {

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

static float aim(glm::vec3 current, float currentAngle, float angularVelocity,
                 glm::vec3 target) {

  glm::mat4 test = glm::lookAt(current, target, glm::vec3(0, 1, 0));

  glm::vec3 dir = glm::normalize(target - current);
  float targetAngle = glm::atan(dir.y, dir.x) - M_PI * 0.5f;
  if (std::abs(currentAngle - targetAngle) > 0.01) {
    currentAngle += glm::sign(targetAngle - currentAngle) * angularVelocity *
                    viewport()->deltaTime;
  }

  return currentAngle;
}

void BaseEnemy::sequenceShoot(EnemyInstance &instance, EnemyClass &cl) {

  instance.shootDelay += viewport()->deltaTime;
  // Shoot
  if (instance.shootDelay > cl.shootThreshold) {

    instance.attackStage++;
    instance.shootDelay = 0.0f;
    if (!((instance.attackStage / cl.attackDivisor) % cl.attackDivisor == 0)) {
      return;
    }
    for (int i = 0; i < cl.shootCount; i++) {
      glm::vec2 shotPosition = cl.getAbsoluteShotCenter(instance);
      glm::vec2 shotDir = cl.shotDirection;
      shotDir += glm::vec2(randf(), randf()) * cl.shootSpread;
      shot->addShot(shotPosition, shotDir, 1, cl.shotSize, cl.shotGravity);
    }
  }
}

void BaseEnemy::sequenceJump(EnemyInstance &instance, EnemyClass &cl) {
  instance.jumpDelay += viewport()->deltaTime;

  if (instance.jumpDelay > cl.jumpThreshold) {
    instance.jumpDelay = 0.0f;

    glm::vec2 diff = targetDifference(instance, cl);
    float t = cl.jumpExpectedTime;

    float vx = diff.x / t;
    float vy = glm::sqrt(2 * diff.y * -cl.gravity.y);

    instance.setVelocity(glm::vec2(vx, vy));
  }
}

void BaseEnemy::sequenceFly(EnemyInstance &instance, EnemyClass &cl) {

  glm::vec2 diff = targetDifference(instance, cl);
  if (glm::abs(diff.y) > cl.flyThreshold) {
    instance.setVelocity(glm::sign(diff.y) * glm::vec2(0.0, cl.flySpeed));
  }
}

void BaseEnemy::sequenceIdle(EnemyInstance &instance, EnemyClass &cl) {

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

glm::vec2 BaseEnemy::targetDifference(EnemyInstance &instance, EnemyClass &cl) {

  glm::vec2 enemy = cl.getAbsoluteShotCenter(instance);
  glm::vec2 player = instance.target->getCombinedMatrix()[3];

  glm::vec2 diff = player - enemy;
  return diff;
}
void BaseEnemy::step(shambhala::StepInfo info) {

  for (int i = 0; i < enemies.size(); i++) {

    // For each enemy instance and class
    EnemyInstance &instance = enemies[i];
    EnemyClass &cl = enemyClasses[instance.enemyClass];

    // Update physical position
    instance.updatePosition(cl.getAcceleration());

    // Update aiming

    glm::vec3 enemyPosition = instance.center->getCombinedMatrix()[3];

    if (cl.aim) {

      glm::vec3 targetPosition = instance.target->getCombinedMatrix()[3];
      instance.aimAngle =
          aim(enemyPosition, instance.aimAngle, cl.aimVelocity, targetPosition);
    }

    // Update enemy
    {
      instance.animationDelay += viewport()->deltaTime;

      if (instance.isAttacking &&
          glm::length(targetDifference(instance, cl)) < cl.attackDistance) {

        if (cl.shot)
          sequenceShoot(instance, cl);
        if (cl.jump)
          sequenceJump(instance, cl);
        if (cl.fly)
          sequenceFly(instance, cl);

        // Update anim
        if (instance.animationDelay > cl.shootingAnimationThreshold) {
          instance.animationIndex++;
          instance.animationIndex =
              instance.animationIndex % cl.attackAnimationCount;
          cl.atlas->configureMaterial(instance.animationIndex +
                                          cl.regularAnimationCount,
                                      instance.model->material);
          instance.animationDelay = 0.0f;
        }

      } else
        sequenceIdle(instance, cl);
    }

    // Update uniforms
    {
      instance.hit -= viewport()->deltaTime;
      instance.hit = glm::max(0.0f, instance.hit);
      instance.model->material->set("hit", instance.hit);
    }
  }
}

void BaseEnemy::editorRender() {
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

void BaseEnemy::editorStep(shambhala::StepInfo info) {

  glm::vec2 pos = ext::rayIntersection(info.mouseRay, ext::zplane(0.0));
  if (viewport()->isRightMousePressed()) {
    spawnEnemy(0, pos.x, pos.y);
  }
}
