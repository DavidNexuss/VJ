#pragma once
#include <shambhala.hpp>
namespace shambhala {
namespace worldmats {
struct Camera : public WorldMaterial {

  glm::vec3 origin;
  glm::vec3 target;

  glm::mat4 viewMatrix;
  glm::mat4 invViewMatrix;
  glm::mat4 projectionMatrix;
  glm::mat4 combinedMatrix;

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
  inline glm::mat4 getCombined() const { return combinedMatrix; }
};

struct DebugCamera : public Camera {

  constexpr static float aproxTime = 0.4f;
  glm::vec3 currentTarget;
  float currentAlpha;
  float currentBeta;

  glm::vec3 nextTarget;
  float nextAlpha;
  float nextBeta;

  float distance;
  float time;

  int cursorStartx, cursorStarty;
  bool pressed;

  float lastAlpha;
  float lastBeta;

  DebugCamera();
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
} // namespace worldmats
} // namespace shambhala
