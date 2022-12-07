#pragma once
#include <array>
namespace geoc {
struct vec2 {
  float x;
  float y;
};

struct vec3 {
  float x, y, z;
};

struct vec4 {
  float x, y, z, w;
};

using vec = vec3;

using mat2 = std::array<vec2, 2>;
using mat3 = std::array<vec3, 3>;
using mat4 = std::array<vec4, 4>;

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
