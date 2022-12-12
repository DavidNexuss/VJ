#include "geoc.hpp"
#include <glm/glm.hpp>
using namespace geoc;

static Intersection noIntersection() { return {{}, 0}; }
static Intersection infiniteIntersection() { return {{}, -1}; }
static vec3 to_vec3(vec2 v) { return vec3{v.x, v.y, 0.0}; }

/** Plane and rays **/

Ray geoc::createRay(mat4 viewMatrix, vec2 position, float ra) {
  float aspectRatio = ra;
  float fov = 3.1415 / 2;

  position = position * 2.0f - vec2(1.0);
  position.x *= aspectRatio;
  position.y = -position.y;

  float zdir = glm::tan(fov / 2);

  glm::vec3 rd = glm::normalize(glm::vec3(position.x, position.y, -1.0f));
  glm::vec3 ro = glm::vec3(position.x, position.y, 0.0);

  glm::mat4 invViewMatrix = glm::inverse(viewMatrix);

  Ray result;
  result.rd = invViewMatrix * glm::vec4(rd, 0.0);
  result.ro = invViewMatrix * glm::vec4(position.x, position.y, -1.0, 1.0);
  return result;
}

float geoc::distanceRayRay(Ray a, Ray b) {
  glm::vec3 b1 = glm::normalize(a.rd);
  glm::vec3 b2 = glm::normalize(b.rd);
  glm::vec3 a1 = a.ro;
  glm::vec3 a2 = b.ro;
  return glm::length(b1 * b2 * (a2 - a1)) / glm::length(glm::cross(b1, b2));
}

Intersection geoc::intersectionRayPlane(Ray ray, Plane a) {
  glm::vec3 planeNormal = a.normal();
  float denom = glm::dot(planeNormal, ray.rd);
  if (glm::abs(denom) > 0.00001f) {
    float t = glm::dot((a.origin - ray.ro), planeNormal) / denom;
    return {{ray.rd * t + ray.ro}, 1};
  }
  return {{glm::vec3(INFINITY)}, 0};
}

glm::vec3 Plane::normal() const { return glm::normalize(glm::cross(x, y)); }

Plane geoc::createPlane(float z) {
  Plane plane;
  plane.x = glm::vec3(1.0, 0.0, 0.0);
  plane.y = glm::vec3(0.0, 1.0, 0.0);
  plane.origin = glm::vec3(0.0, 0.0, z);
  return plane;
}

/** Circumferences ***/

Intersection geoc::intersectionCircumferenceCircumference(Circumference a,
                                                          Circumference b) {

  vec c1 = a.center;
  vec c2 = b.center;
  float d = glm::length(glm::vec2(c1.x - c2.x, c1.y - c2.y));

  if (d > a.radius + b.radius) { // Too far apart
    return noIntersection();
  } else if (d == 0 && a.radius == b.radius) { // Concide intersection
    return infiniteIntersection();
  } else if (d + glm::min(a.radius, b.radius) <
             glm::max(a.radius, b.radius)) { // Subset situation
    return noIntersection();
  } else {
    float A = (a.radius * a.radius - b.radius * b.radius + d * d) / (2.0 * d);
    float H = sqrt(a.radius * a.radius - A * A);
    vec2 p2(c1.x + (A * (c2.x - c1.x) / d), c1.y + (A * (c2.y - c1.y) / d));
    vec2 intersectA =
        vec2(p2.x + (H * (c2.y - c1.y) / d), p2.y - (H * (c2.x - c1.x) / d));
    vec2 intersectB =
        vec2(p2.x - (H * (c2.y - c1.y) / d), p2.y + (H * (c2.x - c1.x) / d));
    if (d == a.radius + b.radius)
      return {{to_vec3(intersectA)}, 1};
    else
      return {{to_vec3(intersectA), to_vec3(intersectB)}, 2};
  }
}
