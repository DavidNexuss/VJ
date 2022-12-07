#include "spriteBatch.hpp"
#include <component/video/texture.hpp>
#include <unordered_map>

struct Font {

  struct Character {
    glm::vec2 size;
    glm::vec2 bearing;
    float advance;
  };

  Font(const char *path, int size);

  inline const Character &getCharacter(char c) { return characters[c]; }
  inline Texture *getTexture() { return glyph; }

private:
  std::vector<Character> characters;
  Texture *glyph;
  bool errored = false;
};

struct FontBatch {

  FontBatch(Font *font, SpriteBatch *batch);
  void renderText(int, const char *textRender, glm::mat4 transformed);

private:
  struct TextSprite {
    std::string text;
    std::vector<Sprite> glyphs;
  };

  TextSprite &buildSprite(int index, const char *text, glm::mat4 transformed,
                          SpriteBatch *spriteBatch);

  std::unordered_map<int, TextSprite> sprites;

  Font *font;
  SpriteBatch *spriteBatch;
};
