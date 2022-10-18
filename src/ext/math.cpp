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

  glm::mat4 invMatrtix = glm::inverse(camera->getCombinedMatrix());

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

float ext::rayDistance(Ray a, Ray b) {
  glm::vec3 b1 = glm::normalize(a.rd);
  glm::vec3 b2 = glm::normalize(b.rd);
  glm::vec3 a1 = a.ro;
  glm::vec3 a2 = b.ro;
  return glm::length(b1 * b2 * (a2 - a1)) / glm::length(glm::cross(b1, b2));
}

glm::vec3 ext::rayIntersection(Ray ray, Plane a) {
  glm::vec3 planeNormal = a.normal();
  float denom = glm::dot(planeNormal, ray.rd);
  if (glm::abs(denom) > 0.00001f) {
    float t = glm::dot((a.origin - ray.ro), planeNormal) / denom;
    return ray.rd * t + ray.ro;
  }
  return glm::vec3(INFINITY);
}

glm::vec3 Plane::normal() const { return glm::normalize(glm::cross(x, y)); }

Plane ext::zplane(float z) {
  Plane plane;
  plane.x = glm::vec3(1.0, 0.0, 0.0);
  plane.y = glm::vec3(0.0, 1.0, 0.0);
  plane.origin = glm::vec3(0.0, 0.0, z);
  return plane;
}
