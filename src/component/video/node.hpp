#pragma once
#include "../base.hpp"
#include "material.hpp"
#include <list>

struct Node : public Material, public BaseComponent<Node> {
  Node *parentNode = nullptr;
  std::list<Node *> children;
  glm::mat4 transformMatrix = glm::mat4(1.0);

  mutable glm::mat4 combinedMatrix;
  mutable bool clean = false;
  mutable bool enableclean = false;

  bool enabled = true;
  bool cachedenabled = true;

public:
  Node();
  bool isEnabled();
  void setEnabled(bool pEnable);
  void setDirty();
  void setParentNode(Node *parent);
  void addChildNode(Node *childNode);
  void setTransformMatrix(const glm::mat4 &newVal);
  void setOffset(glm::vec3 offset);
  void transform(const glm::mat4 &newval);
  const glm::mat4 &getTransformMatrix() const;
  const glm::mat4 &getCombinedMatrix() const;
  void bind(Program *activeProgram) override;
};
