#include "collectable.hpp"
#include "../globals.hpp"
#include "ext.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"

using namespace shambhala;

Collectable::Collectable(CollectableDefinition def) {
  this->def = def;
  mesh = util::createTexturedQuad();
  program = loader::loadProgram("programs/tiled.fs", "programs/tiled.vs");
  material = shambhala::createMaterial();
  texture = def.texture;
  zIndex = 10;

  material->set("mul", glm::vec4(1.2));
  material->set("add", glm::vec4(0.0));
  material->set("base", texture);
  material->set("stOffset", glm::vec2(0.0));
  material->set("stScale", glm::vec2(1.0));
}

Collision Collectable::inside(glm::vec2 position) {
  glm::vec2 start = getNode()->getCombinedMatrix()[3];
  AABB box{start, start + glm::vec2(1.0)};

  Collision col;
  if (box.inside(position))
    col.typeClass = COLLISION_COLLECTABLE;
  return col;
}

void Collectable::signalHit(Collision col) {
  if (getNode()->isEnabled()) {
    def.action(joc::player);
    getNode()->setEnabled(false);
  }
}

Collectable *Collectable::force() {
  CollectableDefinition def;
  def.texture = loader::loadTexture("textures/force.png", 4);
  def.action = [](Player *player) { player->activateForce(true); };
  return new Collectable{def};
}
