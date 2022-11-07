#pragma once
#include "entity.hpp"
#include "player.hpp"
#include <functional>
#include <shambhala.hpp>

using CollectAction = std::function<void(Player *)>;

struct CollectableDefinition {
  CollectAction action;
  shambhala::Texture *texture;
};

struct Collectable : public Entity, public shambhala::Model {

  Collectable(CollectableDefinition definition);
  Collision inside(glm::vec2 position) override;
  void signalHit(Collision col) override;

  static Collectable *force();

private:
  shambhala::Texture *texture;
  CollectableDefinition def;
};
