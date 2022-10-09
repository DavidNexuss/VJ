#include "math.hpp"
using namespace shambhala;
using namespace ext;
using namespace worldmats;

Ray ext::createRay(Camera *camera, glm::vec2 position) {
  glm::mat4 perspectiveMatrix = camera->getProjectionMatrix();
  glm::mat4 viewMatrix = camera->getViewMatrix();

  float fov = M_PI / 2;
  float zdir = glm::tan(fov / 2);

  glm::vec3 rd = glm::normalize(glm::vec3(position.x, position.y, -zdir));
  glm::vec3 ro = glm::vec3(position.x, position.y, 0.0);

  glm::mat4 invViewMatrix = glm::inverse(viewMatrix);

  Ray result;
  result.rd = invViewMatrix * glm::vec4(rd, 0.0);
  result.ro = invViewMatrix * glm::vec4(ro, 1.0);
  return result;
}

float ext::rayDistance(Ray a, Ray b) {}
