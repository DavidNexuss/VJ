#include "shambhala.hpp"
#include "spriteBatch.hpp"
#include <unordered_map>

struct Font {

  struct Character {
    glm::vec2 size;
    glm::vec2 bearing;
    float advance;
  };

  Font(const char *path, int size);

  inline const Character &getCharacter(char c) { return characters[c]; }
  inline shambhala::Texture *getTexture() { return glyph; }

private:
  std::vector<Character> characters;
  shambhala::Texture *glyph;
  bool errored = false;
};

struct FontBatch {

  FontBatch(Font *font);
  void renderText(int, const char *textRender, glm::mat4 transformed);

  Buffer *getVertexBuffer();
  void debug();

private:
  struct TextSprite {
    std::string text;
    std::vector<Sprite> glyphs;
  };

  TextSprite &buildSprite(int index, const char *text, glm::mat4 transformed,
                          SpriteBatch &spriteBatch);

  std::unordered_map<int, TextSprite> sprites;

  SpriteBatch spriteBatch;
  Font *font;
};
