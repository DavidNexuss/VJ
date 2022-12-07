#pragma once
#include "graph.hpp"
#include "linear.hpp"
#include <array>
#include <vector>

namespace geoc {

struct Intersection {
  std::array<vec, 10> intersectionPoints;
  int solutionCount;
};

struct Orientation {
  bool strictInside();
  bool edgeInside();
  bool outside();

  bool left();
  bool right();
};

struct Circumference {
  vec center;
  float radius;
};

struct ParametricLine {
  vec start;
  vec direction;
};

struct Point {
  vec position;
};

struct ConvexPolygon {
  std::vector<vec> vertices;
};

using Triangle = ConvexPolygon;

struct Ray {
  vec3 ro, rd;
};

struct Plane {
  vec3 x;
  vec3 y;
  vec3 origin;

  vec3 normal() const;
};

Triangle createEnclosingTriangle(Circumference &circ);
Circumference createInscribedCircumference(vec a, vec b, vec c);
Circumference createCircumferenceCloud(std::vector<vec> &pointCloud);
ConvexPolygon createConvexPolygonCloud(std::vector<vec> &pointcloud);
Triangle createEnclosingTriangle(std::vector<vec> &pointcloud);
Ray createRay(mat4 viewMatrix, vec2 position, float ra);
Plane createPlane(float z);

float distanceRayRay(Ray a, Ray b);
Intersection intersectionRayPlane(Ray ray, Plane a);

Intersection intersectionCircumferenceCircumference(Circumference a,
                                                    Circumference b);

} // namespace geoc
