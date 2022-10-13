#pragma once
#include <glm/glm.hpp>
namespace shambhala {
struct Ray {
  glm::vec3 ro, rd;
};

struct Plane {
  glm::vec3 x;
  glm::vec3 y;
  glm::vec3 origin;

  glm::vec3 normal() const;
};
} // namespace shambhala
