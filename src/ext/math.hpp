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
} // namespace ext
} // namespace shambhala
