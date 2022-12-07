#pragma once
#include <array>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace geoc {
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat2 = glm::mat2;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using vec = vec3;

vec2 norm(vec2);
vec2 cross(vec2);
vec2 dot(vec2, vec2 other);

vec3 norm(vec3);
vec3 cross(vec3, vec3);
vec3 dot(vec3, vec3);

vec2 transComponent(mat2);
vec3 transComponent(mat3);
vec4 transComponent(mat4);

mat2 rot(float angle);
mat4 rotx(float angle);
mat4 roty(float angle);
mat4 rotz(float angle);
mat4 rot(float angle, vec3 axis);

mat3 trans(vec2 offset);
mat4 trans(vec3 offset);
mat3 scale(vec2 scale);
mat4 scale(vec3 sclae);

mat4 viewMatrix();
mat4 projectionMatrix();

} // namespace geoc
