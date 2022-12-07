#include "vertexPool.hpp"
#include <glm/glm.hpp>

struct SpriteBatch;
struct Sprite {

  glm::vec2 stOffset = glm::vec2(0.0);
  glm::vec2 stScale = glm::vec2(1.0);
  glm::vec2 size = glm::vec2(0.0);
  glm::vec2 position = glm::vec2(0.0);
  float rotation = 0.0;
  int texture = 0;
  glm::vec4 add = glm::vec4(0.0);
  glm::vec4 mul = glm::vec4(1.0);

  void upload();

private:
  size_t instanceId;
  SpriteBatch *batch;
  friend SpriteBatch;
};

struct SpriteBatch {

  Sprite createSprite();
  void destroySprite(Sprite &spr);
  void debug();

  Buffer *getVertexBuffer();

private:
  struct SpriteVertex {
    glm::vec2 position;
    glm::vec2 uv;
    glm::vec4 tintAdd;
    glm::vec4 tintMul;
    int tetxureUnit;
  };

  VertexPool pool;

  void uploadSprite(Sprite &spr);
  friend Sprite;
};
