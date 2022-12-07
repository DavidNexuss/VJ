#include "node.hpp"
#include "program.hpp"

Node::Node() { clean = false; }

void Node::setDirty() {
  clean = false;
  enableclean = false;
  for (Node *child : children) {
    child->setDirty();
  }
}
void Node::addChildNode(Node *childNode) {
  children.push_back(childNode);
  childNode->setDirty();
}
void Node::setParentNode(Node *parentNode) {
  if (this->parentNode) {
    this->parentNode->children.remove(this);
  }
  this->parentNode = parentNode;
  parentNode->addChildNode(this);
}

void Node::setOffset(glm::vec3 offset) {
  transformMatrix[3] = glm::vec4(offset, 1.0);
  setDirty();
}
void Node::setTransformMatrix(const glm::mat4 &newVal) {
  transformMatrix = newVal;
  setDirty();
}

void Node::transform(const glm::mat4 &newval) {
  glm::mat4 oldMatrix = getTransformMatrix();
  setTransformMatrix(newval * oldMatrix);
}

const glm::mat4 &Node::getTransformMatrix() const { return transformMatrix; }

const glm::mat4 &Node::getCombinedMatrix() const {
  if (clean)
    return combinedMatrix;
  clean = true;
  if (parentNode != nullptr)
    return combinedMatrix = parentNode->getCombinedMatrix() * transformMatrix;
  else
    return combinedMatrix = transformMatrix;
}

void Node::bind(Program *activeProgram) {
  activeProgram->bind(Standard::uTransformMatrix, Uniform(getCombinedMatrix()));
}

bool Node::isEnabled() {
  if (enableclean)
    return cachedenabled;

  enableclean = true;
  if (!enabled)
    return cachedenabled = false;
  if (parentNode == nullptr)
    return cachedenabled = true;
  return cachedenabled = parentNode->isEnabled();
}

void Node::setEnabled(bool pEnable) {
  enabled = pEnable;
  setDirty();
}
