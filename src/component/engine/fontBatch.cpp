#include "fontBatch.hpp"
#include "shambhala.hpp"
#include <cstring>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#ifdef DEBUG
#define FT(x)                                                                  \
  {                                                                            \
    errored = x;                                                               \
    if (errored) {                                                             \
      std::cout << "BitmapFontGenerator > " << (#x) << " " << errored          \
                << std::endl;                                                  \
      return;                                                                  \
    }                                                                          \
  }
#else
#define FT(x)                                                                  \
  { x; }
#endif
using namespace shambhala;
Font::Font(const char *path, int fontSize) {
  [&]() {
    FT_Library ft;
    FT(FT_Init_FreeType(&ft));

    FT_Face face;
    FT(FT_New_Face(ft, path, 0, &face));

    FT_Set_Pixel_Sizes(face, 0, fontSize);

    int imageWidth = (fontSize + 2) * 16;
    int imageHeight = (fontSize + 2) * 8;

    uint8_t *buffer = new uint8_t[imageWidth * imageHeight];
    memset(buffer, 0, imageWidth * imageHeight);

    characters.resize(128);
    int maxUnderBaseline = 0;
    FT_UInt glyphIndex;

    for (int i = 32; i < 127; ++i) {
      glyphIndex = FT_Get_Char_Index(face, i);

      FT(FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))

      const FT_Glyph_Metrics &glyphMetrics = face->glyph->metrics;

      int glyphHang = (glyphMetrics.horiBearingY - glyphMetrics.height) / 64;
      if (glyphHang < maxUnderBaseline) {
        maxUnderBaseline = glyphHang;
      }
    }

    for (int i = 0; i < 128; ++i) {
      glyphIndex = FT_Get_Char_Index(face, i);

      FT(FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT));
      FT(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL));

      characters[i].advance = face->glyph->advance.x / float(imageWidth);

      characters[i].size =
          glm::vec2(face->glyph->bitmap.width / float(imageWidth),
                    face->glyph->bitmap.rows / float(imageHeight));

      characters[i].bearing =
          glm::vec2(face->glyph->bitmap_left / float(imageWidth),
                    face->glyph->bitmap_top / float(imageHeight));

      int x = (i % 16) * (fontSize + 2);
      int y = (i / 16) * (fontSize + 2);
      x += 1; // 1 pixel padding from the left side of the tile
      y += (fontSize + 2) - face->glyph->bitmap_top + maxUnderBaseline - 1;

      const FT_Bitmap &bitmap = face->glyph->bitmap;
      for (int xx = 0; xx < bitmap.width; ++xx) {
        for (int yy = 0; yy < bitmap.rows; ++yy) {
          unsigned char r = bitmap.buffer[(yy * (bitmap.width) + xx)];
          buffer[(y + yy) * imageWidth + (x + xx)] = r;
        }
      }
    }

    glyph = shambhala::createTexture();
    glyph->minFilter = GL_LINEAR_MIPMAP_LINEAR;
    glyph->magFilter = GL_LINEAR;

    TextureResource *resource = new TextureResource;
    resource->width = imageWidth;
    resource->height = imageHeight;
    resource->textureBuffer = buffer;
    resource->components = 1;

    glyph->addTextureResource(resource);
  }();
}
void FontBatch::renderText(int idx, const char *textRender,
                           glm::mat4 transformed) {

  FontBatch::TextSprite sprite =
      buildSprite(idx, textRender, transformed, spriteBatch);

  if (sprite.text != textRender) {
    sprites.erase(idx);
    for (int i = 0; i < sprite.glyphs.size(); i++) {
      spriteBatch->destroySprite(sprite.glyphs[i]);
    }
    sprite = buildSprite(idx, textRender, transformed, spriteBatch);
  }
}

FontBatch::TextSprite &FontBatch::buildSprite(int indx, const char *text,
                                              glm::mat4 transformed,
                                              SpriteBatch *spriteBatch) {
  std::string txt = text;
  auto it = sprites.find(indx);
  if (it != sprites.end())
    return it->second;

  auto &batch = sprites[indx];
  batch.text = text;
  batch.glyphs.resize(txt.size());
  for (int i = 0; i < txt.size(); i++) {
    const auto &c = font->getCharacter(txt[i]);
    Sprite spr = spriteBatch->createSprite();
    spr.position = transformed[3];
    spr.size = transformed * glm::vec4(glm::vec3(c.size, 0.0), 1.0);
    spr.stScale = c.size;
    spr.stOffset = glm::vec2(0.0);
    spr.upload();
    batch.glyphs.push_back(spr);
  }

  return batch;
}
