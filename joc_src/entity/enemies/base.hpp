#include "../entity.hpp"
#include "../shot.hpp"
#include "shambhala.hpp"
#include <glm/common.hpp>
#include <unordered_map>

struct EnemyInstance : public PhsyicalObject {
  int enemyClass;
  float hit = 0.0f;
  bool isAttacking = true;
  float animationDelay = 0.0;
  float shootDelay = 0.0;
  float jumpDelay = 0.0;
  int animationIndex;
  float health;

  shambhala::Model *model;
  shambhala::Node *center;

  float aimAngle;
  shambhala::Node *target;

  int pendingShoots;
};

struct EnemyClass {
  DynamicPartAtlas *atlas;
  glm::vec2 shotCenter = glm::vec2(0.0);
  glm::vec2 gravity = glm::vec2(0.0, -0.5);
  glm::vec2 shotGravity = glm::vec2(0.0, -0.3);
  glm::vec2 shotDirection = glm::vec2(-0.5, 0.5) * 4.0f;
  glm::mat4 scaleTransform = glm::mat4(1.0);

  float mass = 2.0;
  float maxHealth = 8.0;

  float attackDistance = 20.0;

  // Shot sequence
  bool shot = false;
  float shotSize = 3.0;
  int shootCount = 4;
  float shootSpread = 0.6;
  float shotDecay = 0.0;
  float shootingAnimationThreshold = 0.8;
  float shootThreshold = 15.0f;

  // Aim and shoot sequence
  bool aim = false;
  float aimVelocity = 1.0;

  // Jump sequence
  bool jump = false;
  float jumpThreshold = 10.0;
  float jumpMaxVelocity = 20.0;
  float jumpExpectedTime = 8.0;

  // Fly and jump
  float fly = false;
  float flyThreshold = 0.5;
  float flySpeed = 0.5;

  // Animations
  int regularPart = 0;
  int attackPart = 0;

  int regularAnimationCount = 6;
  int attackAnimationCount = 4;
  float regularAnimationThreshold = 0.3;

  inline glm::vec2 getAbsoluteShotCenter(EnemyInstance &instance) {
    return instance.model->getNode()->getCombinedMatrix() *
           glm::vec4(shotCenter, 0.0, 1.0);
  };

  inline glm::vec2 getAcceleration() { return gravity; }
};

struct BaseEnemy : public shambhala::LogicComponent,
                   public EntityComponent,
                   public Entity {

  BaseEnemy(ShotComponent *shot);

  void createEnemyClass(int id, EnemyClass enemyClass);
  int spawnEnemy(int id, float x, float y);
  void killEnemy(int index);

  void step(shambhala::StepInfo info) override;
  void editorStep(shambhala::StepInfo info) override;
  void editorRender() override;

  Collision inside(glm::vec2 position) override;
  void signalHit(Collision col) override;

  shambhala::Node *target;

private:
  int castPoint(glm::vec2 position);
  ShotComponent *shot;
  std::unordered_map<int, EnemyClass> enemyClasses;
  simple_vector<EnemyInstance> enemies;

  void sequenceFly(EnemyInstance &, EnemyClass &);
  void sequenceShoot(EnemyInstance &, EnemyClass &);
  void sequenceIdle(EnemyInstance &, EnemyClass &);
  void sequenceJump(EnemyInstance &, EnemyClass &);
  glm::vec2 targetDifference(EnemyInstance &instance, EnemyClass &cl);
};
