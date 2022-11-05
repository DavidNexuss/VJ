#include "playerCamera.hpp"
#include "imgui.h"
#include "shambhala.hpp"
#include <fstream>
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

using namespace std;
static float offset = 12;

PlayerCamera::PlayerCamera(const char *configurationFile,
                           shambhala::Node *node) {

  this->target = node;

  std::ifstream file(configurationFile);
  if (file) {

    int cameraCount;
    file >> cameraCount;
    for (int i = 0; i < cameraCount; i++) {
      PlayerCameraWaypoint waypoint;
      file >> waypoint.x;
      file >> waypoint.y;
      file >> waypoint.z;
      file >> waypoint.zoom;
      file >> waypoint.startx;
      file >> waypoint.starty;
      std::string interpmode;

      file >> interpmode;
      if (interpmode == "cos") {
        waypoint.interp = InterpolationMode::COSINE;
      }
      if (interpmode == "lin") {
        waypoint.interp = InterpolationMode::LINEAR;
      }

      waypoints.push_back(waypoint);
    }
  }

  this->waypointIndex = 0;
  this->currentWaypoint = waypoints[0];
}

void PlayerCamera::step(shambhala::StepInfo info) {

  float currentTime = 0.0f;
  static float targetTime = 3.0f;

  glm::vec3 playerPosition =
      this->target->getCombinedMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0);

  glm::vec3 targetPosition = getCombinedMatrix() *
                             this->target->getCombinedMatrix() *
                             glm::vec4(0.0, 0.0, 0.0, 1.0);

  if (targetPosition.x <= -10.0) {
    currentWaypoint.x -= shambhala::viewport()->deltaTime;
  }

  if (targetPosition.x >= 2.0) {
    currentWaypoint.x += shambhala::viewport()->deltaTime;
  }

  if (waypointIndex < (waypoints.size() - 1)) {
    if (playerPosition.x > waypoints[waypointIndex].startx) {
      float t = glm::cos((currentTime / targetTime) * M_PI / 2.0f);
      currentWaypoint.zoom =
          waypoints[waypointIndex].zoom * t + waypoints[waypointIndex + 1].zoom;
      currentTime += shambhala::viewport()->deltaTime;
    }

    if (targetTime >= currentTime) {
      waypointIndex = waypointIndex + 1;
      currentWaypoint.zoom = waypoints[waypointIndex].zoom;
    }
  }

  glm::vec3 eye = glm::vec3(currentWaypoint.x, currentWaypoint.y,
                            currentWaypoint.z + offset);
  glm::vec3 target = glm::vec3(currentWaypoint.x, currentWaypoint.y, offset);

  glm::mat4 viewMatrix = glm::lookAtLH(eye, target, glm::vec3(0, 1, 0));

  setViewMatrix(viewMatrix);
  setProjectionMatrix(
      glm::perspective(glm::radians(90.0f * currentWaypoint.zoom),
                       float(shambhala::viewport()->getScreenWidth()) /
                           float(shambhala::viewport()->getScreenHeight()),
                       0.01f, 500.0f));
}

void PlayerCamera::editorRender() {
  if (ImGui::Begin("PlayerCamera")) {
    ImGui::InputFloat3("Position", &this->currentWaypoint.x);
    ImGui::InputFloat("Offset", &offset);
    ImGui::End();
  }
}
