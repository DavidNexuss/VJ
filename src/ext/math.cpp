#include "math.hpp"
#include "shambhala.hpp"
using namespace shambhala;
using namespace ext;
using namespace worldmats;

Ray ext::createRay(Camera *camera, glm::vec2 position) {
  float aspectRatio = viewport()->aspectRatio();

  position = position * 2.0f - glm::vec2(1.0);
  position.x *= aspectRatio;
  position.y = -position.y;

  glm::mat4 perspectiveMatrix = camera->getProjectionMatrix();
  glm::mat4 viewMatrix = camera->getViewMatrix();

  glm::mat4 invMatrtix = glm::inverse(camera->combinedMatrix);

  float fov = M_PI / 2;
  float zdir = glm::tan(fov / 2);

  glm::vec3 rd = glm::normalize(glm::vec3(position.x, position.y, -1.0f));
  glm::vec3 ro = glm::vec3(position.x, position.y, 0.0);

  glm::mat4 invViewMatrix = glm::inverse(viewMatrix);

  Ray result;
  result.rd = invViewMatrix * glm::vec4(rd, 0.0);
  result.ro = invViewMatrix * glm::vec4(position.x, position.y, -1.0, 1.0);
  return result;
}

float ext::rayDistance(Ray a, Ray b) {}
