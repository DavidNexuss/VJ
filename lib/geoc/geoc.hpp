#pragma once
#include "graph.hpp"
#include "linear.hpp"
#include <array>
#include <vector>

namespace geoc {

struct Intersection {
  std::array<vec, 10> intersectionPoints;

  int solutionCount();
  vec solution(int index);
};

struct Orientation {
  bool strictInside();
  bool edgeInside();
  bool outside();

  bool left();
  bool right();
};

struct GeometricObject {
  virtual Intersection intersect(GeometricObject &other) {
    return Intersection{};
  }
  virtual Orientation orientation(GeometricObject &other) {
    return Orientation{};
  }
  virtual float distance() { return 0.0; }
};

struct Circumference : public GeometricObject {
  vec center;
  float radius;

  Intersection intersect(GeometricObject &other) override;
  float distance() override;
};

struct ParametricLine : public GeometricObject {
  vec start;
  vec direction;
  Intersection intersect(GeometricObject &other) override;
  float distance() override;
};

struct Point : public GeometricObject {
  vec position;
  Intersection intersect(GeometricObject &other) override;
  float distance() override;
};

struct ConvexPolygon : public GeometricObject {
  std::vector<vec> vertices;
  Intersection intersect(GeometricObject &other) override;
  float distance() override;
};

struct Triangle : public GeometricObject {};
Triangle createEnclosingTriangle(Circumference &circ);
Circumference createInscribedCircumference(vec a, vec b, vec c);
Circumference createCircumferenceCloud(std::vector<vec> &pointCloud);
ConvexPolygon createConvexPolygonCloud(std::vector<vec> &pointcloud);
Triangle createEnclosingTriangle(std::vector<vec> &pointcloud);

} // namespace geoc
