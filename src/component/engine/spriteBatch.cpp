#include "spriteBatch.hpp"
#include "ext/util.hpp"

using namespace shambhala;

Sprite SpriteBatch::createSprite() {
  Sprite spr;
  spr.instanceId = pool.acquireBucket(sizeof(SpriteVertex) * 6);
  spr.batch = this;
  return spr;
}

void SpriteBatch::destroySprite(Sprite &spr) {
  pool.releaseBucket(spr.instanceId);
}

void SpriteBatch::uploadSprite(Sprite &spr) {
  uint8_t *buffer = pool.getVertex(spr.instanceId);
  SpriteVertex *vertices = (SpriteVertex *)buffer;
  vertices[0].tetxureUnit = spr.texture;
  vertices[0].tintAdd = spr.add;
  vertices[0].tintMul = spr.mul;

  glm::mat4 tr = util::scale(spr.size.x, spr.size.y, 0.0) *
                 util::rotate(0, 0, 1, spr.rotation) *
                 util::translate(-0.5, -0.5, 0.0);
  glm::vec2 positions[4];
  glm::vec2 uv[4];

  positions[0] = tr * glm::vec4(0.0, 0.0, 0.0, 1.0);
  positions[1] = tr * glm::vec4(0.0, 1.0, 0.0, 1.0);
  positions[2] = tr * glm::vec4(1.0, 1.0, 0.0, 1.0);
  positions[3] = tr * glm::vec4(1.0, 0.0, 0.0, 1.0);

  uv[0] = spr.stOffset + spr.stScale * glm::vec2(0.0, 0.0);
  uv[1] = spr.stOffset + spr.stScale * glm::vec2(0.0, 1.0);
  uv[2] = spr.stOffset + spr.stScale * glm::vec2(1.0, 1.0);
  uv[3] = spr.stOffset + spr.stScale * glm::vec2(1.0, 0.0);

  static int indices[] = {0, 1, 2, 2, 3, 0};
  for (int i = 0; i < 6; i++) {
    vertices[i] = vertices[0];
    vertices[i].uv = uv[indices[i]];
    vertices[i].position = positions[indices[i]];
  }

  pool.signalUpdate(spr.instanceId);
}

void Sprite::upload() { batch->uploadSprite(*this); }

Buffer *SpriteBatch::getVertexBuffer() { return pool.getBufferObject(); }
