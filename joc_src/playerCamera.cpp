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
      file >> waypoint.cameraspeed;
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

  static float targetTime = 3.0f;

  glm::vec3 playerPosition = this->target->getCombinedMatrix()[3];

  glm::vec3 targetPosition = getCombinedMatrix() *
                             this->target->getCombinedMatrix() *
                             glm::vec4(0.0, 0.0, 0.0, 1.0);

  if (targetPosition.x >= 2.0) {
    currentWaypoint.x += shambhala::viewport()->deltaTime * 1.5;
  }

  currentWaypoint.x +=
      shambhala::viewport()->deltaTime * currentWaypoint.cameraspeed;

  glm::vec2 wayPosition{currentWaypoint.x, currentWaypoint.y};

  if (waypointIndex < (waypoints.size() - 1)) {
    if (playerPosition.x > waypoints[waypointIndex + 1].startx) {

      float t = glm::cos((interptime / targetTime) * M_PI / 2.0f);

      currentWaypoint.zoom = waypoints[waypointIndex].zoom * t +
                             waypoints[waypointIndex + 1].zoom * (1 - t);

      wayPosition.x += (waypoints[waypointIndex + 1].x) * (1 - t);
      wayPosition.y += (waypoints[waypointIndex + 1].y) * (1 - t);

      interptime += shambhala::viewport()->deltaTime;
    }

    if (interptime >= targetTime) {
      waypointIndex = waypointIndex + 1;

      currentWaypoint.zoom = waypoints[waypointIndex].zoom;
      currentWaypoint.cameraspeed = waypoints[waypointIndex].cameraspeed;

      currentWaypoint.x += waypoints[waypointIndex].x;
      currentWaypoint.y += waypoints[waypointIndex].y;
      interptime = 0.0;
    }
  }

  glm::vec3 eye =
      glm::vec3(wayPosition.x, wayPosition.y, currentWaypoint.z + offset);
  glm::vec3 target = glm::vec3(wayPosition.x, wayPosition.y, offset);

  glm::mat4 viewMatrix = glm::lookAtLH(eye, target, glm::vec3(0, 1, 0));
  glm::mat4 projectionMatrix =

      glm::perspective(glm::radians(90.0f * currentWaypoint.zoom),
                       float(shambhala::viewport()->getScreenWidth()) /
                           float(shambhala::viewport()->getScreenHeight()),
                       0.01f, 500.0f);
  setViewMatrix(viewMatrix);
  setProjectionMatrix(projectionMatrix);

  glm::vec2 transformedPlayer =
      getCombinedMatrix() * glm::vec4(playerPosition, 1.0);

  if (targetPosition.x < -40.0) {
    outside = true;
  }
}

void PlayerCamera::editorRender() {
  if (ImGui::Begin("PlayerCamera")) {
    ImGui::InputFloat3("Position", &this->currentWaypoint.x);
    ImGui::InputFloat("Offset", &offset);
    ImGui::InputFloat("Zoom", &currentWaypoint.zoom);

    ImGui::Separator();

    for (int i = 0; i < waypoints.size(); i++) {
      ImGui::Text("Waypoint %d", i);
      ImGui::InputFloat3("Position %d", &this->waypoints[i].x);
      ImGui::InputFloat("Zoom", &this->waypoints[i].zoom);
    }
    ImGui::End();
  }
}
