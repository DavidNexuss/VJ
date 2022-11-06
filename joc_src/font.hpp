#include <shambhala.hpp>

struct BitMapFont {

  BitMapFont(const char *texturePath);
  void render(const std::string &text, glm::vec2 position, float size);
  void render(const std::string &text, glm::vec2 position, glm::vec2 size);

private:
  shambhala::Program *program;
  shambhala::Mesh *mesh;
  shambhala::Texture *fonttexture;
};
