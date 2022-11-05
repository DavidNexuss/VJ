#include <shambhala.hpp>

struct BitMapFont {

  BitMapFont(const char *texturePath);
  void render(const std::string &text, glm::vec2 position, float size);

private:
  shambhala::Program *program;
  shambhala::Mesh *mesh;
  shambhala::Texture *fonttexture;
};
