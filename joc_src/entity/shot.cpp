#include "shot.hpp"
#include "ext/math.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
#include <ext.hpp>
#include <vector>
using namespace shambhala;

ShotComponent::ShotComponent() {
  mesh = util::createTexturedQuad();
  node = shambhala::createNode();
  program = loader::loadProgram("programs/shot.fs", "programs/instanced.vs");
  material = shambhala::createMaterial();
  node->setName("ShotNode");
  node->transform(util::translate(-0.5, -0.5, 0.2));
  zIndex = 2;
  setName("ShotComponent");
}

static int maximumShots = 500;
void ShotComponent::addShot(glm::vec2 position, glm::vec2 direction, int type) {
  shot_positions.push({position, 0.0});
  shot_velocity.push({direction, 0.0});
  shots.push(Shot{type});
  needsUniformFlush = true;

  if (shot_positions.size() > maximumShots) {
    shot_positions.removeNShift(0);
    shot_velocity.removeNShift(0);
    shots.removeNShift(0);
  }
}

void ShotComponent::step(shambhala::StepInfo info) {

  // Physics update
  for (int i = 0; i < shots.size(); i++) {
    shot_positions[i] += shot_velocity[i] * viewport()->deltaTime;
  }

  static std::vector<int> hitted;
  hitted.clear();
  for (int i = 0; i < shots.size(); i++) {
    bool hit = false;
    for (int j = 0; j < entities.size(); j++) {
      Entity *ent = entities[j];
      if (ent->inside(glm::vec2(shot_positions[i]))) {
        ent->signalHit();
        if (!hit) {
          hitted.push_back(i);
          hit = true;
        }
      }
    }
  }

  std::sort(hitted.begin(), hitted.end(),
            [&](int lhs, int rhs) { return lhs > rhs; });

  for (int i = 0; i < hitted.size(); i++) {
    int shot = hitted[i];
    shot_positions.removeNShift(shot);
    shot_velocity.removeNShift(shot);
    shots.removeNShift(shot);
  }
  this->instance_count = shot_positions.size();
  needsUniformFlush = true;
}

void ShotComponent::uniformFlush() {
  {
    Uniform positions;
    positions.type = shambhala::VEC3PTR;
    positions.VEC3PTR = &shot_positions[0];
    positions.count = this->instance_count;
    device::useUniform("uPositionOffset", positions);
  }
}
void ShotComponent::draw() {
  glDisable(GL_DEPTH_TEST);
  if (this->instance_count != 0) {
    device::useProgram(program);
    device::useMesh(mesh);
    device::useMaterial(node);

    if (needsUniformFlush) {
      uniformFlush();
      needsUniformFlush = false;
    }

    device::drawCall(*this);
  }
  glEnable(GL_DEPTH_TEST);
}

void ShotComponent::editorStep(shambhala::StepInfo info) {

  if (viewport()->isRightMousePressed()) {
    glm::vec3 intersection =
        ext::rayIntersection(info.mouseRay, ext::zplane(0.0));
    int n = 10;
    for (int i = 0; i < n; i++) {
      float p = i / float(n);
      float x = glm::cos(p * M_PI * 2);
      float y = glm::sin(p * M_PI * 2);
      addShot(glm::vec2(intersection.x, intersection.y), glm::vec2(x, y), 1);
    }
  }
}
