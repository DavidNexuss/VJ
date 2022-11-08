#include "globals.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
using namespace shambhala;

void joc::renderBox(glm::vec2 start, glm::vec2 end, glm::vec4 color) {
  static Program *tiled =
      loader::loadProgram("programs/tiled.fs", "programs/tiled.vs");

  static Mesh *mesh = util::createTexturedQuad();

  glm::vec2 dim = end - start;

  glm::mat4 tr =
      util::translate(start.x, start.y, 0.0) * util::scale(dim.x, dim.y, 1.0);

  tiled->use();
  tiled->bind(Standard::uTransformMatrix, tr);
  tiled->bind(Standard::uViewMatrix, glm::mat4(1.0));
  tiled->bind(Standard::uProjectionMatrix, glm::mat4(1.0));
  mesh->use();
  mesh->use();

  shambhala::drawCall();
}
