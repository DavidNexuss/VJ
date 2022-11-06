#include "forceShot.hpp"
#include "../globals.hpp"
#include "imgui.h"
#include "shambhala.hpp"

using namespace shambhala;
ForceShotComponent::ForceShotComponent() {
  this->forceProgram =
      loader::loadProgram("programs/force.fs", "programs/force.vs");
  this->mesh = shambhala::createMesh();
}
void ForceShotComponent::addShot(glm::vec2 start, glm::vec2 velocity,
                                 glm::vec3 tint, float duration) {
  ForceShot shot;
  shot.lastingDuration = duration;
  shot.buffer = new shambhala::VertexBuffer;
  shot.position = start;
  shot.velocity = velocity;
  shot.buffer->attributes =
      simple_vector<VertexAttribute>{{0, 2}, {1, 1}, {2, 1}};
  this->shots.push(shot);
}

struct ForceShotVertex {
  glm::vec2 position;
  float emmitTime;
  float color;
};

void ForceShotComponent::step(shambhala::StepInfo info) {

  for (int i = 0; i < shots.size(); i++) {
    if (shots[i].lastingDuration <= 0.0) {
      // delete[] shots[i].buffer;
      shots.removeNShift(i);
      i--;
      continue;
    }

    glm::vec2 lp = shots[i].position;
    shots[i].position += shambhala::viewport()->deltaTime * shots[i].velocity;
    glm::vec2 np = shots[i].position;

    Collision col;
    bool rebota = false;
    for (int j = 0; j < entities.size(); j++) {
      col = entities[j]->inside(shots[i].position);
      if (col.typeClass == COLLISION_ENEMY) {
        entities[j]->signalHit(col);
      } else if (!col.isEmpty())
        rebota = true;
    }

    // Do rebota

    shots[i].lastingDuration -= shambhala::viewport()->deltaTime;

    int vboindex =
        shots[i].buffer->vertexBuffer.size() / sizeof(ForceShotVertex);
    shots[i].buffer->vertexBuffer.resize(shots[i].buffer->vertexBuffer.size() +
                                         sizeof(ForceShotVertex) * 6);

    ForceShotVertex *buff =
        (ForceShotVertex *)&shots[i].buffer->vertexBuffer[0] + vboindex;

    auto emitVertex = [](ForceShotVertex *buff, glm::vec2 position) {
      buff->position = position;
      buff->emmitTime = joc::clock->getTime();
      buff->color = 1;
    };

    emitVertex(buff, lp - glm::vec2(0.0, 0.1));
    emitVertex(buff + 1, lp + glm::vec2(0.0, 0.1));
    emitVertex(buff + 2, np + glm::vec2(0.0, 0.1));
    emitVertex(buff + 3, np + glm::vec2(0.0, 0.1));
    emitVertex(buff + 4, lp - glm::vec2(0.0, 0.1));
    emitVertex(buff + 5, np - glm::vec2(0.0, 0.1));

    shots[i].buffer->signalUpdate();
  }
}

void ForceShotComponent::render() {

  for (int i = 0; i < shots.size(); i++) {

    if (shots[i].buffer->vertexBuffer.size() == 0)
      continue;

    this->forceProgram->use();
    this->forceProgram->bind(Standard::uTransformMatrix, glm::mat4(1.0));

    this->mesh->vbo = shots[i].buffer;
    shots[i].buffer->use();
    this->mesh->use();

    shambhala::device::drawCall();
  }
}

#include "imgui.h"
void ForceShotComponent::editorRender() {

  if (ImGui::Begin("Force")) {

    ImGui::Checkbox("DebugSpwan", &this->debugSpawn);
    ImGui::End();

    if (this->debugSpawn) {
      addShot(glm::vec2(-20.0, 5.0), glm::vec2(2.0, 0.3), glm::vec3(20.0),
              10.0);
      this->debugSpawn = false;
    }
  }
}
