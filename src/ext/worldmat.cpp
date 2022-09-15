
#include "worldmat.hpp"
#include "shambhala.hpp"
#include <glm/ext.hpp>
#include <standard.hpp>

using namespace glm;
using namespace std;
using namespace shambhala;
using namespace shambhala::worldmats;

Camera::Camera() {
  fov = 90.0f;
  zNear = 0.1f;
  zFar = 500.0f;

  useZoom = false;
  l = b = -1.0f;
  r = t = 1.0f;
  zmin = -2.0;
  zmax = 2.0;
  zoomSpeed = 0.0;
  zoomFactor = 1.0;

  useOrthographic = false;
}
glm::mat4 Camera::createProjectionMatrix() {

  float zoom = useZoom ? zoomFactor : 1.0f;

  if (useZoom) {
    const auto deltaTime = 0.1f;
    zoomSpeed += viewport()->scrollY * deltaTime;
    zoomFactor -= zoomSpeed * deltaTime;
    zoomSpeed -= zoomSpeed * deltaTime * zoomDamping;
    zoomFactor = std::max(zoomFactor, 0.0f);
  }
  return glm::perspective(glm::radians(fov * zoom),
                          float(shambhala::viewport()->screenWidth) /
                              float(shambhala::viewport()->screenHeight),
                          zNear, zFar);
}
glm::mat4 Camera::createOrthoMatrix() {
  return glm::ortho(l, r, b, t, zmin, zmax);
}

void Camera::defaultViewMatrix() {
  glm::vec3 computeOrigin = origin;
  if (parentNode.valid())
    computeOrigin =
        parentNode->getTransformMatrix() * glm::vec4(computeOrigin, 1.0);
  glm::mat4 viewMatrix = glm::lookAt(computeOrigin, target, glm::vec3(0, 1, 0));

  setViewMatrix(viewMatrix);
}
void Camera::defaultProjectionMatrix() {
  setProjectionMatrix(useOrthographic ? createOrthoMatrix()
                                      : createProjectionMatrix());
}

void Camera::setParentNode(NodeID parentNode) { this->parentNode = parentNode; }
void Camera::updateMatrices() {
  combinedMatrix = projectionMatrix * viewMatrix;
  invViewMatrix = glm::inverse(viewMatrix);
}

void Camera::lookAt(const glm::vec3 &origin, const glm::vec3 &target) {
  this->origin = origin;
  this->target = target;
}

void Camera::update(float deltaTime) {
  defaultProjectionMatrix();
  defaultViewMatrix();
  updateMatrices();
}

void Camera::bind(Program *current) {

  if (parentNode != nullptr) {
    defaultViewMatrix();
  }
  device::useUniform(Standard::uProjectionMatrix, Uniform(projectionMatrix));
  current.bindUniform(Standard::uProjectionMatrix, Uniform(projectionMatrix));
  current.bindUniform(Standard::uViewPos, Uniform(glm::vec3(invViewMatrix[3])));

  if (current.getMaterial()->isSkyBoxMaterial) {
    mat4 skyView = mat4(mat3(viewMatrix));
    current.bindUniform(Standard::uViewMatrix, Uniform(skyView));
  } else
    current.bindUniform(Standard::uViewMatrix, Uniform(viewMatrix));
}
