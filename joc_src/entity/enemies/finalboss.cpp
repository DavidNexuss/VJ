#include "finalboss.hpp"
#include "ext.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
#include <cstdio>
using namespace shambhala;

static glm::vec2 textureSize = glm::vec2(640.0f, 1864.0f);
static glm::vec2 stScalePart = glm::vec2(36.0f) / textureSize;
static float bodyStart = 48.0 * 3 / textureSize.y;
static float speed = 0.5;

FinalBoss::FinalBoss() {
  texture = loader::loadTexture("textures/boss.png", 4);
  program = loader::loadProgram("programs/tiled.fs", "programs/tiled.vs");
  mesh = util::createTexturedQuad();

  for (int i = 0; i < 30; i++) {
    parts.push(FinalBossPart{-float(i) * 0.50f});
  }
  rootNode = shambhala::createNode();
  texture->useNeareast = true;
}

void FinalBoss::draw() {
  if (active) {
    program->use();
    mesh->use();

    program->bind("base", texture);
    for (int i = 0; i < parts.size(); i++) {
      Path path = getPath(parts[i].step + globalStep);
      parts[i].lastMat = path.getCollisionTransform();
      parts[i].lastRotMat = path.getTransform();
      ST st = getST(path);

      program->bind(Standard::uTransformMatrix,
                    rootNode->getTransformMatrix() * path.getTransform());

      float t = parts[i].damage / 2.0;
      program->bind("add", glm::vec4(0.0));
      program->bind("mul", glm::vec4(1.0, 1.0 - t, 1.0 - t, 1.0));
      program->bind("stOffset", st.offset);
      program->bind("stScale", st.scale);

      shambhala::device::drawCall();
    }
  }
}

Collision FinalBoss::inside(glm::vec2 position) {
  Collision col{};

  for (int i = 0; i < parts.size(); i++) {
    AABB box{parts[i].lastMat[3],
             parts[i].lastMat * glm::vec4(glm::vec3(1.0, 1.0, 0.0), 1.0)};

    if (box.inside(position)) {
      Collision col;
      col.typeClass = COLLISION_ENEMY;
      col.typeInstance = i;
      return col;
    }
  }
  return col;
}

void FinalBoss::attackSequence1(float t) {
  static float lastT = 0.0;

  for (int i = 0; i < parts.size(); i++) {
    float d = parts[i].step + t;
    parts[i].shoot = true;
    parts[i].shotdelay = 2.0 + parts[i].step;
  }
}

void FinalBoss::signalHit(Collision col) {
  for (int i = 0; i < parts.size(); i++) {
    parts[i].damage = 1.4;
  }
  parts[col.typeInstance].damage = 2.0;
  health -= col.damage;
}

FinalBoss::Path FinalBoss::Path::interpolate(Path start, Path end, float t) {

  t = glm::cos(glm::min(1.0f, glm::max(0.0f, t)) * M_PI * 0.5f);

  FinalBoss::Path finalPath;
  finalPath.position += start.position * t + end.position * (1.0f - t);
  finalPath.velocity += start.velocity * t + end.velocity * (1.0f - t);
  return finalPath;
}

FinalBoss::Path FinalBoss::getPath(float f) {

  float s1 = 5.0;
  float s2 = 15.0;

  FinalBoss::Path paths[3];
  paths[0].position = glm::vec2{f, glm::sin(f) + 2.0} * 5.0f;
  paths[0].velocity = glm::vec2{speed, glm::cos(f)};

  paths[1].position = glm::vec2{glm::cos(f) + s1, glm::sin(f) + 2.0} * 5.0f;
  paths[1].velocity = glm::vec2{glm::sin(f), glm::cos(f)};

  paths[2].position = glm::vec2{glm::cos(f) + s1 + 0.5, f - 40.0} * 5.0f;
  paths[2].velocity = glm::vec2{glm::sin(f), speed};

  float interpolation[2];
  interpolation[0] = f >= s1 ? (f - s1) * 0.5f : 0.0;
  interpolation[1] = f >= s2 ? (f - s2) * 0.5f : 0.0;

  FinalBoss::Path finalPath;

  finalPath = paths[0];
  if (f >= s1 && f < 40.0) {
    finalPath =
        FinalBoss::Path::interpolate(paths[0], paths[1], (f - s1) * 0.5f);
  }

  if (f >= 40.0) {
    finalPath =
        FinalBoss::Path::interpolate(paths[1], paths[2], (f - s2) * 0.5f);
  }

  return finalPath;
}
glm::mat4 FinalBoss::Path::getTransform() {
  return util::translate(position.x, position.y, 0.0) *
         util::rotate(0, 0, 1, glm::atan(velocity.y, velocity.x)) *
         util::scale(5.0) * util::translate(-0.5f, -0.5f, 0.0);
}

glm::mat4 FinalBoss::Path::getCollisionTransform() {
  return util::translate(position.x, position.y, 0.0) * util::scale(5.0) *
         util::translate(-0.5f, -0.5f, 0.0);
}

FinalBoss::ST FinalBoss::getST(Path p) {
  float angle = glm::atan(p.velocity.y, p.velocity.x) / (2 * M_PI);
  int step = glm::floor(angle * 16.0f);

  float offset = stScalePart.y * glm::floor((glm::sin(p.t) * 0.5 + 0.5) * 5.0);

  FinalBoss::ST st;
  st.scale = stScalePart;
  st.offset = glm::vec2(0, bodyStart) + glm::vec2(0, offset);
  return st;
}

void FinalBoss::step(shambhala::StepInfo info) {
  if (active) {
    globalStep += viewport()->deltaTime * speed;

    if (globalStep > 80.0)
      globalStep = 0.0;

    for (int i = 0; i < parts.size(); i++) {
      parts[i].damage = glm::max(0.0f, parts[i].damage - viewport()->deltaTime);
    }

    for (int i = 0; i < parts.size(); i++) {
      if (parts[i].shoot) {
        if (parts[i].shotdelay < 0.0) {
          shot->addShot(parts[i].lastRotMat[3], parts[i].lastRotMat[1], 1, 3);
          parts[i].shoot = false;
        }

        parts[i].shotdelay -= viewport()->deltaTime;
      }
    }

    shootDelta -= viewport()->deltaTime;

    if (shootDelta < 0.0) {
      attackSequence1(globalStep);
      shootDelta = std::max(10.0, health / 10.0);
    }
  }
}
