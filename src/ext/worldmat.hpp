#pragma once
#include <component/logicHook.hpp>
#include <component/video/material.hpp>
#include <component/video/node.hpp>

namespace worldmats {

struct SimpleCamera : public Material {

  SimpleCamera();

  void setViewMatrix(const glm::mat4 &viewMatrix);
  void setProjectionMatrix(const glm::mat4 &projectionMatrix);

  inline glm::mat4 getViewMatrix() { return viewMatrix; }
  inline glm::mat4 getProjectionMatrix() { return projectionMatrix; }
  inline glm::mat4 getCombinedMatrix() { return combinedMatrix; }
  inline glm::mat4 getInvViewMatrix() { return invViewMatrix; }

  inline bool cull(glm::vec3 point) {
    glm::vec4 p = getCombinedMatrix() * glm::vec4(point, 1.0);
    return p.x >= -1.0 && p.x <= 1.0 && p.y >= -1.0 && p.y <= 1.0;
  }

private:
  void updateMatrices();
  glm::mat4 viewMatrix = glm::mat4(1.0f);
  glm::mat4 invViewMatrix = glm::mat4(1.0f);
  glm::mat4 projectionMatrix = glm::mat4(1.0f);
  glm::mat4 combinedMatrix = glm::mat4(1.0f);
};

struct Camera : public SimpleCamera, public Logic {

  float fov;
  float zoomDamping = 0.6;
  float zoomFactor;
  float zoomSpeed;
  float zNear;
  float zFar;
  bool useZoom;
  bool useOrthographic;

  float l, r, b, t, zmin, zmax;

  void defaultProjectionMatrix();
  void defaultViewMatrix();
  void setParentNode(Node *parentNode);
  void lookAt(const glm::vec3 &position, const glm::vec3 &target);

  Camera();

  void step(StepInfo info) override;

private:
  glm::mat4 createViewMatrix();
  glm::mat4 createProjectionMatrix();
  glm::mat4 createOrthoMatrix();
  glm::vec3 origin = glm::vec3(0.0), target = glm::vec3(0.0);

  Node *parentNode = nullptr;
};

struct DebugCamera : public Camera {

  DebugCamera();
  void step(StepInfo info) override;

private:
  glm::vec3 currentTarget = glm::vec3(0.0);
  float currentAlpha = 1.0;
  float currentBeta = 0.0;

  glm::vec3 nextTarget = glm::vec3(0.0);
  float nextAlpha = 1.0;
  float nextBeta = 0.0;

  float distance = 10.0;
  float time = 0.0;

  int cursorStartx = 0, cursorStarty = 0;
  bool pressed = false;
  bool middlepressed = false;

  float lastAlpha = 0.0;
  float lastBeta = 0.0;

  glm::vec3 lastTarget = glm::vec3(0.0);
  glm::vec3 moveOffset = glm::vec3(0.0);
};

class FlyCamera : public Camera {
  glm::vec3 position;
  glm::vec3 velocity;

  float velocityDamping;
  int advanceKey;

public:
  FlyCamera();
};

struct Camera2D : public SimpleCamera, public Logic {

  Camera2D();
  glm::vec2 offset = glm::vec2(0.0);
  float zoom = 1.0f;

  void step(StepInfo info) override;
};

struct Clock : public Material, public Logic {
  Clock();
  void step(StepInfo info) override;

  inline float getTime() { return globalTime; }

private:
  float globalTime = 0.0f;
};
} // namespace worldmats
