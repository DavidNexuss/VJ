
#include "worldmat.hpp"
#include "shambhala.hpp"
#include <glm/ext.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <standard.hpp>

using namespace glm;
using namespace std;
using namespace shambhala;
using namespace shambhala::worldmats;

SimpleCamera::SimpleCamera() {}

void SimpleCamera::setViewMatrix(const glm::mat4 &viewMatrix) {
  this->viewMatrix = viewMatrix;
  updateMatrices();

  set(Standard::uViewMatrix, viewMatrix);
  set(Standard::uViewPos, glm::vec3(invViewMatrix[3]));
}
void SimpleCamera::setProjectionMatrix(const glm::mat4 &projectionMatrix) {
  this->projectionMatrix = projectionMatrix;
  updateMatrices();

  set(Standard::uProjectionMatrix, projectionMatrix);
}
void SimpleCamera::updateMatrices() {
  this->combinedMatrix = projectionMatrix * viewMatrix;
  this->invViewMatrix = glm::inverse(viewMatrix);
}
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
  needsFrameUpdate = true;
  hasCustomBindFunction = true;
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
  if (parentNode != nullptr)
    computeOrigin =
        parentNode->getTransformMatrix() * glm::vec4(computeOrigin, 1.0);
  glm::mat4 viewMatrix = glm::lookAt(computeOrigin, target, glm::vec3(0, 1, 0));

  setViewMatrix(viewMatrix);
}
void Camera::defaultProjectionMatrix() {
  setProjectionMatrix(useOrthographic ? createOrthoMatrix()
                                      : createProjectionMatrix());
}

void Camera::setParentNode(Node *parentNode) { this->parentNode = parentNode; }
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
  device::useUniform(Standard::uViewPos, Uniform(glm::vec3(invViewMatrix[3])));

  if (current->hint_skybox) {
    mat4 skyView = mat4(mat3(viewMatrix));
    device::useUniform(Standard::uViewMatrix, Uniform(skyView));
  } else
    device::useUniform(Standard::uViewMatrix, Uniform(viewMatrix));
}

void Camera::setViewMatrix(const glm::mat4 &_viewMatrix) {
  viewMatrix = _viewMatrix;
}
void Camera::setProjectionMatrix(const glm::mat4 &_projectionMatrix) {
  projectionMatrix = _projectionMatrix;
}

glm::vec3 createViewDir(float a, float b) {
  glm::vec3 viewDir;
  viewDir.x = cos(a) * cos(b);
  viewDir.y = sin(b);
  viewDir.z = sin(a) * cos(b);
  return viewDir;
}

void DebugCamera::update(float deltatime) {
  time += deltatime;
  if (time >= aproxTime) {
    time = 0.0f;
    currentTarget = nextTarget;
    currentBeta = nextBeta;
    currentAlpha = nextAlpha;
  }

  auto setDir = [&](float a, float b) {
    time = 0.0f;
    currentAlpha += lastAlpha;
    currentBeta += lastBeta;
    nextAlpha = a;
    nextBeta = b;
    lastAlpha = 0.0f;
    lastBeta = 0.0f;
  };

  if (viewport()->isKeyJustPressed(KEY_KP_1)) {
    setDir(0.0f, 0.0f);
  }

  if (viewport()->isKeyJustPressed(KEY_KP_3)) {
    setDir(M_PI, 0.0f);
  }

  if (viewport()->isKeyJustPressed(KEY_KP_4)) {
    setDir(M_PI * 0.5f, 0.0f);
  }

  if (viewport()->isKeyJustPressed(KEY_KP_6)) {
    setDir(-M_PI * 0.5f, 0.0f);
  }

  if (viewport()->isKeyJustPressed(KEY_KP_7)) {
    setDir(M_PI * 0.5f, -M_PI * 0.5f);
  }

  if (viewport()->isKeyJustPressed(KEY_KP_9)) {
    setDir(M_PI * 0.5f, M_PI * 0.5f);
  }

  // Mouse input
  if (shambhala::input_mouse_free()) {
    bool mousePressed = viewport()->isMousePressed();
    bool middleMousePressed = viewport()->isMiddleMousePressed();

    distance += viewport()->scrollY * std::max(std::abs(distance * 0.05), 0.1);
    viewport()->scrollY = 0.0f;

    if (mousePressed) {
      if (!pressed) {
        pressed = true;
        cursorStartx = viewport()->xpos;
        cursorStarty = viewport()->ypos;
      }

      lastAlpha =
          ((viewport()->xpos - cursorStartx) / viewport()->screenWidth) * M_PI *
          2.0;
      lastBeta =
          ((viewport()->ypos - cursorStarty) / viewport()->screenHeight) *
          M_PI * 2.0;
    }
    if (!mousePressed && pressed) {
      pressed = false;
      currentAlpha += lastAlpha;
      nextAlpha += lastAlpha;

      currentBeta += lastBeta;
      nextBeta += lastBeta;

      lastAlpha = 0.0f;
      lastBeta = 0.0f;
    }

    if (middleMousePressed) {
      if (!middlepressed) {
        middlepressed = true;
        cursorStartx = viewport()->xpos;
        cursorStarty = viewport()->ypos;
        lastTarget = nextTarget;
      }

      glm::vec2 difference =
          glm::vec2(cursorStartx - viewport()->xpos,
                    viewport()->ypos - cursorStarty) /
          glm::vec2(viewport()->screenWidth, viewport()->screenHeight);

      glm::vec3 offset =
          invViewMatrix * glm::vec4(glm::vec3(difference, 0.0), 0.0);

      nextTarget = lastTarget + offset * distance;
    }

    if (!middleMousePressed && middlepressed) {
      middlepressed = false;
    }
  }

  glm::vec3 currentDirection =
      createViewDir(currentAlpha + lastAlpha, currentBeta + lastBeta);
  glm::vec3 nextDirection =
      createViewDir(nextAlpha + lastAlpha, nextBeta + lastBeta);

  // Interpolation
  float p = time / aproxTime;
  glm::vec3 interpDirection =
      createViewDir(currentAlpha * (1.0f - p) + nextAlpha * p + lastAlpha,
                    currentBeta * (1.0f - p) + nextBeta * p + lastBeta);
  glm::vec3 target = currentTarget * (1.0f - p) + nextTarget * p;
  glm::vec3 viewpos = target - interpDirection * distance;

  lookAt(viewpos, target);
  Camera::update(deltatime);
}

Camera2D::Camera2D() {
  hasCustomBindFunction = true;
  Material::needsFrameUpdate = true;
}

void Camera2D::bind(Program *activeProgram) {
  static glm::mat4 viewMatrix = glm::mat4(1.0);
  device::useUniform(Standard::uProjectionMatrix, cameraMatrix);
  device::useUniform(Standard::uViewMatrix, viewMatrix);
}

void Camera2D::update(float deltatime) {
  float width = shambhala::viewport()->screenWidth * 0.5;
  float height = shambhala::viewport()->screenHeight * 0.5;

  width /= 500.0f;
  height /= 500.0f;
  cameraMatrix = glm::ortho(zoom * -width, zoom * width, zoom * -height,
                            zoom * height, -1.0f, 1.0f);
  cameraMatrix =
      glm::translate(cameraMatrix, glm::vec3(-offset.x, -offset.y, 0.0));
}

Clock::Clock() {
  hint_is_material = this;
  set("uTime", 0.0f);
}
void Clock::step(StepInfo info) {
  globalTime += viewport()->deltaTime;
  set("uTime", globalTime);
}
