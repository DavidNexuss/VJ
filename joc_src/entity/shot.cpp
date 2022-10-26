#include "shot.hpp"
#include "device/shambhala_audio.hpp"
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

  laserStart = audio::createSoundMesh();
  laserStart->soundResource.acquire(
      audio::createSoundResource("joc2d/sound/laser_start.wav"));
  laserEnd = audio::createSoundMesh();
  laserEnd->soundResource.acquire(
      audio::createSoundResource("joc2d/sound/laser_end.wav"));
  soundNode = shambhala::createNode();
}

static int maximumShots = 500;
void ShotComponent::addShot(glm::vec2 position, glm::vec2 direction, int type,
                            float sizeoffset, glm::vec2 acceleration) {
  shot_positions.push({position, 0.0});
  shot_velocity.push({direction, 0.0});
  shot_acceleration.push({acceleration, 0.0});
  shot_scale.push({sizeoffset});
  shots.push(Shot{type});
  needsUniformFlush = true;

  if (shot_positions.size() > maximumShots) {
    shot_positions.removeNShift(0);
    shot_velocity.removeNShift(0);
    shots.removeNShift(0);
    shot_scale.removeNShift(0);
    shot_acceleration.removeNShift(0);
  }

  if (type == 0) {
    soundNode->setOffset(glm::vec3(position, 0.0));
    audio::SoundModel model;
    model.mesh = laserStart;
    model.node = soundNode;
    model.pitch = startPitch;
    model.play();
  }
}

void ShotComponent::step(shambhala::StepInfo info) {

  // Physics update
  for (int i = 0; i < shots.size(); i++) {
    shot_velocity[i] += shot_acceleration[i] * viewport()->deltaTime;
  }

  for (int i = 0; i < shots.size(); i++) {
    shot_positions[i] += shot_velocity[i] * viewport()->deltaTime;
  }

  static std::vector<int> hitted;
  hitted.clear();
  for (int i = 0; i < shots.size(); i++) {
    bool hit = false;
    for (int j = 0; j < entities.size(); j++) {
      Entity *ent = entities[j];
      Collision col = ent->inside(glm::vec2(shot_positions[i]));
      if (!col.isEmpty()) {
        Collision chit = col;
        chit.velocity = shot_velocity[i];
        chit.damage = shot_scale[i];
        chit.typeClass =
            shots[i].type == 0 ? COLLISION_PLAYER_SHOT : COLLISION_ENEMY_SHOT;
        if (chit.typeClass == COLLISION_PLAYER_SHOT &&
            col.typeClass == COLLISION_PLAYER)
          continue;
        if (chit.typeClass == COLLISION_ENEMY_SHOT &&
            col.typeClass == COLLISION_ENEMY)
          continue;

        ent->signalHit(chit);

        if (!hit) {
          hitted.push_back(i);
          hit = true;
        }

        if (shots[i].type == 0) {
          soundNode->setOffset(shot_positions[i]);
          audio::SoundModel model;
          model.mesh = laserStart;
          model.node = soundNode;
          model.pitch = endPitch;
          model.play();
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
    shot_scale.removeNShift(shot);
    shot_acceleration.removeNShift(shot);
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

  {
    Uniform type;
    type.type = shambhala::INTPTR;
    type.INTPTR = (int *)&shots[0];
    type.count = this->instance_count;
    device::useUniform("type", type);
  }

  {
    Uniform size;
    size.type = shambhala::FLOATPTR;
    size.FLOATPTR = &shot_scale[0];
    size.count = this->instance_count;
    device::useUniform("uSizeOffset", size);
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

#include <imgui.h>

void ShotComponent::editorRender() {
  if (ImGui::Begin("Shot")) {
    ImGui::InputFloat("Start Pitch", &this->startPitch);
    ImGui::InputFloat("End Pitch", &this->endPitch);
    ImGui::End();
  }
}
