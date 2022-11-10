#include "vertexPool.hpp"
#include <glm/glm.hpp>
#include <shambhala.hpp>

struct SpriteBatch;
struct Sprite {

  void setSTOffset(glm::vec2 offset);
  void setSTScale(glm::vec2 scale);
  void setSize(glm::vec2 size);
  void setPosition(glm::vec2 position);
  void setRotation(float rotation);
  void setTexture(int unit);
  void setAdditive(glm::vec4 add);
  void setMultiplicative(glm::vec4 mul);

private:
  size_t instanceId;
  SpriteBatch *batch;
  friend SpriteBatch;
};

struct SpriteBatch {

  Sprite createSprite();
  void destroySprite(Sprite &spr);

private:
  struct SpriteVertex {
    glm::vec2 position;
    glm::vec2 uv;
    glm::vec4 tintAdd;
    glm::vec4 tintMul;
    int tetxureUnit;
  };

  VertexPool pool;

  void uploadSprite(int instanceId, Sprite &spr);
  friend Sprite;
};
