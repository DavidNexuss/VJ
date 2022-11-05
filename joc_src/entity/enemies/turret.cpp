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
