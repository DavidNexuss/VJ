#include "spriteBatch.hpp"

using namespace shambhala;

Sprite SpriteBatch::createSprite() {
  Sprite spr;
  spr.instanceId = pool.acquireBucket(sizeof(SpriteVertex) * 6);

  return Sprite{};
}

void SpriteBatch::destroySprite(Sprite &spr) {}

void SpriteBatch::uploadSprite(int instanceId, Sprite &spr) {}
