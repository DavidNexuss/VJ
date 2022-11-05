#include "font.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
using namespace shambhala;

static VertexBuffer vbo;
BitMapFont::BitMapFont(const char *path) {
  program = loader::loadProgram("programs/font.fs", "programs/font.vs");
  fonttexture = loader::loadTexture(path, 4);
  fonttexture->useNeareast = true;
  fonttexture->clamp = true;
  mesh = shambhala::createMesh();
  vbo.attributes = {{Standard::aPosition, 3}, {Standard::aUV, 2}};
}
static char asciitolower(char in) {
  if (in <= 'Z' && in >= 'A')
    return in - ('Z' - 'z');
  return in;
}

struct FontVertex {
  glm::vec3 position;
  glm::vec2 st;
};

void BitMapFont::render(const std::string &text, glm::vec2 position,
                        float size) {

  simple_vector<FontVertex> positions;

  static int gliphSizex = 18;
  static int gliphSizey = 16;
  static int jumpline = 9;

  static float px = 18.0f / 162.0f;
  static float py = 16.0f / 128.0f;

  static float startx = 0;
  static float starty = py * 3.0;

  positions.resize(text.size() * 6);

  for (int i = 0; i < positions.size(); i += 6) {
    char c = asciitolower(text[i / 6]);
    int charIndex = (c - 'a') + (c >= 'h') + (c >= 'r');
    int x = charIndex % jumpline;
    int y = charIndex / jumpline;

    int j = i / 6;
    positions[i].st = {x * px + startx, (y + 1) * py + starty};
    positions[i].position =
        glm::vec3{j * px * size + position.x, position.y, 0};

    positions[i + 1].st = {x * px + startx, y * py + starty};
    positions[i + 1].position =
        glm::vec3{j * px * size + position.x, position.y + py * size, 0};

    positions[i + 2].st = {(x + 1) * px + startx, y * py + starty};
    positions[i + 2].position = glm::vec3{
        j * px * size + position.x + px * size, position.y + py * size, 0};

    positions[i + 3].st = {(x + 1) * px + startx, y * py + starty};
    positions[i + 3].position = glm::vec3{
        j * px * size + position.x + px * size, position.y + py * size, 0};

    positions[i + 4].st = {(x + 1) * px + startx, (y + 1) * py + starty};
    positions[i + 4].position =
        glm::vec3{j * px * size + position.x + px * size, position.y, 0};

    positions[i + 5].st = {x * px + startx, (y + 1) * py + starty};
    positions[i + 5].position =
        glm::vec3{j * px * size + position.x, position.y, 0};
  }

  mesh->vbo = &vbo;
  mesh->vbo->vertexBuffer = positions.drop();
  mesh->vbo->signalUpdate();
  mesh->vbo->mode = GL_STREAM_DRAW;

  mesh->use();
  program->use();
  program->bind(Standard::uTransformMatrix, glm::mat4(1.0));
  program->bind("base", fonttexture);

  shambhala::device::drawCall();
}
