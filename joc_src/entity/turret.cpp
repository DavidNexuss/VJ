#include "turret.hpp"
#include "ext/math.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
#include <glm/ext/matrix_transform.hpp>
using namespace shambhala;

Turret::Turret(DynamicPartAtlas *atlas) {
  this->model = atlas->createDynamicPart(97);
  this->model->getNode()->transform(shambhala::util::scale(2.0));

  shambhala::addModel(this->model);

  {
    this->turret_emission = atlas->createLight();
    this->turret_emission->getNode()->setParentNode(this->model->getNode());

    {
      glm::mat4 light = util::scale(5.0) * util::translate(-0.5, -0.25, 0.0) *
                        glm::inverse(model->getNode()->getTransformMatrix());

      light[3][2] = 0.001;

      this->turret_emission->getNode()->setTransformMatrix(light);
    }

    this->turret_emission->material = shambhala::createMaterial();

    turret_emission->material->set("uv_offset", glm::vec2(0.0));
    turret_emission->material->set("uv_scale", glm::vec2(1.0));
    turret_emission->material->set("tint_color", glm::vec3(1.0));

    this->turret_emission->zIndex = 2;
  }

  shambhala::addModel(this->turret_emission);

  originalMatrix = this->model->getNode()->getTransformMatrix();
}

void Turret::attach(shambhala::Node *attachNode) {
  if (this->model->node == nullptr)
    this->model->node = shambhala::createNode();
  this->model->node->setParentNode(attachNode);
}

void Turret::target(shambhala::Node *node) { this->target_node = node; }

void Turret::step(shambhala::StepInfo info) {
  if (target_node == nullptr)
    return;
  glm::mat4 current = model->getNode()->getCombinedMatrix();
  glm::mat4 target = target_node->getCombinedMatrix();

  glm::mat4 test = glm::lookAt(glm::vec3(current[3]), glm::vec3(target[3]),
                               glm::vec3(0, 1, 0));

  glm::vec3 dir = glm::normalize(target[3] - current[3]);
  targetAngle = glm::atan(dir.y, dir.x) - M_PI * 0.5f;
  if (std::abs(currentAngle - targetAngle) > 0.01) {
    currentAngle += glm::sign(targetAngle - currentAngle) * angularVelocity *
                    viewport()->deltaTime;
  }

  glm::mat4 newMat =
      shambhala::util::rotate(0.0, 0.0, 1.0, currentAngle) * originalMatrix;
  newMat[3] = glm::vec4(current[3]);
  newMat[3][2] = -0.001;
  model->getNode()->setTransformMatrix(newMat);
  // TODO: Do somehting here
}

void Turret::editorStep(StepInfo info) {
  if (viewport()->isRightMousePressed()) {
    glm::vec3 point = ext::rayIntersection(info.mouseRay, ext::zplane(0.0f));
    glm::mat4 transform = this->model->getNode()->getTransformMatrix();
    transform[3] = glm::vec4(point, 1.0);
    this->model->getNode()->setTransformMatrix(transform);
  }
}
