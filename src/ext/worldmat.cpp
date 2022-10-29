
#include "worldmat.hpp"
#include "shambhala.hpp"
#include <glm/ext.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <standard.hpp>

using namespace glm;
using namespace std;
using namespace shambhala;
using namespace shambhala::worldmats;

SimpleCamera::SimpleCamera() { hint_isCamera = true; }

void SimpleCamera::setViewMatrix(const glm::mat4 &viewMatrix) {
  this->viewMatrix = viewMatrix;
  updateMatrices();

  set(Standard::uViewMatrix, glm::mat4(viewMatrix));
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
}
glm::mat4 Camera::createProjectionMatrix() {

  float zoom = useZoom ? zoomFactor : 1.0f;

  if (useZoom) {
    const auto deltaTime = 0.1f;
    zoomSpeed += viewport()->getScrolllY() * deltaTime;
    zoomFactor -= zoomSpeed * deltaTime;
    zoomSpeed -= zoomSpeed * deltaTime * zoomDamping;
    zoomFactor = std::max(zoomFactor, 0.0f);
  }
  return glm::perspective(glm::radians(fov * zoom),
                          float(shambhala::viewport()->getScreenWidth()) /
                              float(shambhala::viewport()->getScreenHeight()),
                          zNear, zFar);
}
glm::mat4 Camera::createOrthoMatrix() {
  return glm::ortho(l, r, b, t, zmin, zmax);
}

glm::mat4 Camera::createViewMatrix() {
  return glm::lookAt(origin, target, glm::vec3(0, 1, 0));
}

void Camera::setParentNode(Node *parentNode) { this->parentNode = parentNode; }

void Camera::lookAt(const glm::vec3 &origin, const glm::vec3 &target) {
  this->origin = origin;
  this->target = target;
}

void Camera::step(StepInfo info) {
  setViewMatrix(createViewMatrix());
  setProjectionMatrix(createProjectionMatrix());
}

static glm::vec3 createViewDir(float a, float b) {
  glm::vec3 viewDir;
  viewDir.x = cos(a) * cos(b);
  viewDir.y = sin(b);
  viewDir.z = sin(a) * cos(b);
  return viewDir;
}

DebugCamera::DebugCamera() { hint_is_material = this; }
void DebugCamera::step(StepInfo info) {
  float deltatime = viewport()->deltaTime;
  time += deltatime;

  constexpr static float aproxTime = 0.4f;
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

    distance +=
        viewport()->getScrolllY() * std::max(std::abs(distance * 0.05), 0.1);

    if (mousePressed) {
      if (!pressed) {
        pressed = true;
        cursorStartx = viewport()->getX();
        cursorStarty = viewport()->getY();
      }

      lastAlpha =
          ((viewport()->getX() - cursorStartx) / viewport()->getScreenWidth()) *
          M_PI * 2.0;
      lastBeta = ((viewport()->getY() - cursorStarty) /
                  viewport()->getScreenHeight()) *
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

    glm::vec2 moveDirection = glm::vec2(0.0, 0.0);
    if (middleMousePressed) {
      if (!middlepressed) {
        middlepressed = true;
        cursorStartx = viewport()->getX();
        cursorStarty = viewport()->getY();
        lastTarget = nextTarget;
      }

      moveDirection = glm::vec2(cursorStartx - viewport()->getX(),
                                viewport()->getY() - cursorStarty) /
                      glm::vec2(viewport()->getScreenWidth(),
                                viewport()->getScreenHeight());
    } else {
      moveDirection.x += viewport()->isKeyPressed(KEY_D) * deltatime * 0.4;
      moveDirection.y += viewport()->isKeyPressed(KEY_W) * deltatime * 0.4;

      moveDirection.x -= viewport()->isKeyPressed(KEY_A) * deltatime * 0.4;
      moveDirection.y -= viewport()->isKeyPressed(KEY_S) * deltatime * 0.4;
      lastTarget = nextTarget;
    }

    glm::vec3 offset =
        getInvViewMatrix() * glm::vec4(glm::vec3(moveDirection, 0.0), 0.0);

    moveOffset += offset * 10.0f;

    // nextTarget = lastTarget + offset * distance;

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

  Camera::lookAt(viewpos + moveOffset, target + moveOffset);
  Camera::step(info);
}

void Camera2D::step(StepInfo info) {
  float width = shambhala::viewport()->getScreenWidth() * 0.5;
  float height = shambhala::viewport()->getScreenHeight() * 0.5;

  width /= 500.0f;
  height /= 500.0f;

  setViewMatrix(
      glm::translate(glm::mat4(1.0f), glm::vec3(-offset.x, -offset.y, 0.0)));
  setProjectionMatrix(glm::ortho(zoom * -width, zoom * width, zoom * -height,
                                 zoom * height, -1.0f, 1.0f));
}

Clock::Clock() {
  hint_is_material = this;
  set("uTime", 0.0f);
}
void Clock::step(StepInfo info) {
  globalTime += viewport()->deltaTime;
  set("uTime", globalTime);
}
