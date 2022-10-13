#pragma once
#include <core/core.hpp>
#include <ext.hpp>
#include <ext/worldmat.hpp>
#include <glm/glm.hpp>

namespace shambhala {
namespace ext {
// origin viewport coordinates from 0 to 1
Ray createRay(worldmats::Camera *camera, glm::vec2 position);
float rayDistance(Ray a, Ray b);
glm::vec3 rayIntersection(Ray ray, Plane a);

Plane zplane(float z);
} // namespace ext
} // namespace shambhala
