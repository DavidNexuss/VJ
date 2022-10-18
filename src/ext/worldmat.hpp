#pragma once
#include <shambhala.hpp>
namespace shambhala {
namespace worldmats {

struct SimpleCamera : public Material {

  SimpleCamera();

  void setViewMatrix(const glm::mat4 &viewMatrix);
  void setProjectionMatrix(const glm::mat4 &projectionMatrix);

  inline glm::mat4 getViewMatrix() { return viewMatrix; }
  inline glm::mat4 getProjectionMatrix() { return projectionMatrix; }
  inline glm::mat4 getCombinedMatrix() { return combinedMatrix; }
  inline glm::mat4 getInvViewMatrix() { return invViewMatrix; }

private:
  void updateMatrices();
  glm::mat4 viewMatrix = glm::mat4(1.0f);
  glm::mat4 invViewMatrix = glm::mat4(1.0f);
  glm::mat4 projectionMatrix = glm::mat4(1.0f);
  glm::mat4 combinedMatrix = glm::mat4(1.0f);
};

struct Camera : public Material {

  glm::vec3 origin = glm::vec3(0, 0, 1);
  glm::vec3 target = glm::vec3(0, 0, 0);

  glm::mat4 viewMatrix = glm::mat4(1.0f);
  glm::mat4 invViewMatrix = glm::mat4(1.0f);
  glm::mat4 projectionMatrix = glm::mat4(1.0f);
  glm::mat4 combinedMatrix = glm::mat4(1.0f);

  Node *parentNode = nullptr;

public:
  float fov;
  float zoomDamping = 0.6;
  float zoomFactor;
  float zoomSpeed;
  float zNear;
  float zFar;
  bool useZoom;
  bool useOrthographic;

  float l, r, b, t, zmin, zmax;

  glm::mat4 createProjectionMatrix();
  glm::mat4 createOrthoMatrix();

  void updateMatrices();

  void setViewMatrix(const glm::mat4 &_viewMatrix);
  void setProjectionMatrix(const glm::mat4 &_projectionMatrix);

  void defaultProjectionMatrix();
  void defaultViewMatrix();
  void setParentNode(Node *parentNode);
  void lookAt(const glm::vec3 &position, const glm::vec3 &target);

  /**
   * @brief updates camera matrices using the currnet camera configuration
   */
  virtual void update(float deltaTime) override;

  /**
   * @brief sends camera uniforms to the shaders
   */
  virtual void bind(Program *program) override;

  inline glm::mat4 getViewMatrix() const { return viewMatrix; }
  inline glm::mat4 getProjectionMatrix() const { return projectionMatrix; }
  inline glm::mat4 getCombined() const { return combinedMatrix; }

  Camera();
};

struct DebugCamera : public Camera {

  constexpr static float aproxTime = 0.4f;
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

  virtual void update(float) override;
};

class FlyCamera : public Camera {
  glm::vec3 position;
  glm::vec3 velocity;

  float velocityDamping;
  int advanceKey;

public:
  virtual void update(float deltaTime) override;

  FlyCamera();
};

struct Camera2D : public Material {

  Camera2D();
  glm::vec2 offset = glm::vec2(0.0);
  float zoom = 1.0f;

  void update(float deltatime) override;
  void bind(Program *activeProgram) override;

private:
  glm::mat4 cameraMatrix;
};

struct Clock : public Material, public LogicComponent {
  Clock();
  void step(StepInfo info) override;

private:
  float globalTime = 0.0f;
};
} // namespace worldmats
} // namespace shambhala
