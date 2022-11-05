#include "ext/worldmat.hpp"
#include "shambhala.hpp"
#include <ext.hpp>
#include <vector>

//Hello
enum class InterpolationMode { LINEAR, COSINE };

struct PlayerCameraWaypoint {
  float startx;
  float starty;
  float x;
  float y;
  float z;
  float zoom;
  float cameraspeed;
  InterpolationMode interp;
};

struct PlayerCamera : public shambhala::worldmats::SimpleCamera,
                      shambhala::LogicComponent {

  PlayerCamera(const char *configurationFile, shambhala::Node *targetNode);
  void step(shambhala::StepInfo info) override;
  void editorRender() override;

private:
  glm::vec3 offsetPosition;
  shambhala::Node *target;
  std::vector<PlayerCameraWaypoint> waypoints;
  PlayerCameraWaypoint currentWaypoint;
  int waypointIndex;
  float interptime = 0.0f;
};
