#include "entity.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
#include <ext.hpp>
#include <glm/geometric.hpp>
using namespace shambhala;

static int max_colision_checks = 10;

void DynamicPartAtlas::configureNode(int part, Node *node) {

  glm::vec2 scale = glm::vec2(coords[part * 4 + 2], coords[part * 4 + 3]);
  glm::mat4 transform = node->getTransformMatrix();
  transform[0].x = scale.x / 32.0f;
  transform[1].y = scale.y / 32.0f;
  node->setTransformMatrix(transform);
}
void DynamicPartAtlas::configureMaterial(int part, Material *material) {
  glm::vec2 textureSize =
      glm::vec2(textureAtlas->textureData[0].cleanFile()->width,
                textureAtlas->textureData[0].cleanFile()->height);

  glm::vec2 offset = glm::vec2(coords[part * 4], coords[part * 4 + 1]);
  glm::vec2 scale = glm::vec2(coords[part * 4 + 2], coords[part * 4 + 3]);
  offset /= textureSize;
  scale /= textureSize;

  material->set("base", textureAtlas);
  material->set("uv_scale", scale);
  material->set("uv_offset", offset);
  material->set("textureSize", textureSize);
}

static Model *createStandardEntity() {
  Model *model = shambhala::createModel();
  model->node = shambhala::createNode();
  model->mesh = util::createTexturedQuad();
  model->zIndex = -1;
  return model;
}
Model *DynamicPartAtlas::createDynamicPart(int part) {
  Model *model = shambhala::createModel();
  model->node = shambhala::createNode();
  model->mesh = util::createTexturedQuad();
  model->material = shambhala::createMaterial();
  model->program = renderingProgram;
  configureMaterial(part, model->material);
  configureNode(part, model->node);
  model->node->setName((std::string("Part ") + std::to_string(part)).c_str());
  model->zIndex = -1;
  return model;
}

Model *DynamicPartAtlas::createLight() {
  Model *model = createStandardEntity();
  model->program =
      loader::loadProgram("programs/light.fs", "programs/regular.vs");
  model->node->setName("Light");
  return model;
}

DynamicPartAtlas *DynamicPartAtlas::create(shambhala::Program *program,
                                           shambhala::Texture *text,
                                           int *corrds) {
  DynamicPartAtlas *atlas = new DynamicPartAtlas;
  atlas->renderingProgram = program;
  atlas->textureAtlas = text;
  atlas->coords = corrds;
  return atlas;
}

static glm::vec2 transformedPosition(glm::vec2 p, Node *node) {
  glm::vec4 result = node->getCombinedMatrix() * glm::vec4(p, 0.0, 1.0);
  return glm::vec2(result);
}

glm::vec2 AABB::corner(int index) {
  glm::vec2 position;
  position.x = (index % 2) == 0 ? lower.x : higher.x;
  position.y = (index / 2) == 0 ? lower.y : higher.y;
  return position;
}
AABB PhsyicalObject::containingBox(AABB originalBox) {

  glm::vec2 a = transformedPosition(originalBox.corner(0), immediateNode);
  glm::vec2 b = transformedPosition(originalBox.corner(1), immediateNode);
  glm::vec2 c = transformedPosition(originalBox.corner(2), immediateNode);
  glm::vec2 d = transformedPosition(originalBox.corner(3), immediateNode);

  glm::vec2 min = glm::min(glm::min(a, b), glm::min(c, d));
  glm::vec2 max = glm::max(glm::max(a, b), glm::max(c, d));
  return AABB{min, max};
}

void PhsyicalObject::setEntityComponent(EntityComponent *comp) {
  component = comp;
}
Collision PhsyicalObject::collisionCheck() {
  AABB playerAABB = containingBox(getLocalBox());
  for (int i = 0; i < component->entities.size(); i++) {
    Collision col =
        component->entities[i]->inside(playerAABB.lower, playerAABB.higher);
    if (!col.isEmpty() && col.typeClass != COLLISION_COLLECTABLE)
      return col;

    if (col.typeClass == COLLISION_COLLECTABLE) {
      component->entities[i]->signalHit(Collision{});
    }
  }
  return Collision{};
}

void PhsyicalObject::updateNodePosition(glm::vec2 nodePosition) {

  positionNode->setOffset(glm::vec3(nodePosition, 0.15));
}

bool PhsyicalObject::updatePositionStep(glm::vec2 acceleration, float delta) {
  velocity += acceleration * viewport()->deltaTime;
  velocity = velocity * damping;

  glm::vec2 oldPlayerPosition = position;
  position += velocity * viewport()->deltaTime;
  updateNodePosition(position);
  Collision col = collisionCheck();
  if (!col.isEmpty()) {

    position = oldPlayerPosition;
    updateNodePosition(position);
    position += glm::vec2(velocity.x, 0.0) * viewport()->deltaTime;
    updateNodePosition(position);
    Collision col = collisionCheck();
    if (!col.isEmpty()) {
      handleCollision(col);
      position = oldPlayerPosition;
      updateNodePosition(oldPlayerPosition);
      return false;
    }
  }
  return true;
}
void PhsyicalObject::updatePosition(glm::vec2 acceleration) {
  bool correct = false;
  float delta = viewport()->deltaTime;
  for (int i = 0; i < max_colision_checks && !correct; i++) {
    correct |= updatePositionStep(acceleration, delta);
    delta = delta * 0.5f;
  }
}
