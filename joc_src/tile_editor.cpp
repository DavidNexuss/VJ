#include "shambhala.hpp"
#include "tile.hpp"
#include <ext.hpp>
using namespace shambhala;

#include <imgui/imgui.h>
void TileMap::editorStep(shambhala::StepInfo info) {

  glm::mat4 transform = model->node->getCombinedMatrix();

  Plane plane;
  plane.x = glm::vec3(transform[0]);
  plane.y = glm::vec3(transform[1]);
  plane.origin = glm::vec3(transform[3]);
  glm::vec3 offset;
  glm::vec2 size = {sizex, sizey};
  size = size * 0.5f;
  offset = plane.x * float(sizex) * 0.5f + plane.y * float(sizey) * 0.5f;

  glEnable(GL_BLEND);
  glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  util::renderPlaneGrid(plane.x, plane.y,
                        plane.origin - glm::vec3(0.0, 0.0, 0.01) + offset,
                        glm::vec4(1.0), size);

  glm::vec3 intersection = ext::rayIntersection(info.mouseRay, plane);
  int x = intersection.x;
  int y = intersection.y;

  if (x >= 0 && y >= 0 && x < sizex && y < sizey) {

    if (viewport()->isRightMousePressed()) {
      set(x, y, this->editorMaterial);
    }

    if (viewport()->isKeyJustPressed(KEY_X)) {
      set(x, y, 0);
    }
  }

  if (viewport()->isKeyJustPressed(KEY_L)) {
    this->editorMaterial++;
  }
}
void TileMap::editorRender() {

  if (ImGui::Begin("Tile")) {
    ImGui::InputInt("Current: ", &this->editorMaterial);

    Tile tile = atlas->getTile(this->editorMaterial);
    glm::vec2 uv = {tile.xstart, tile.ystart};
    glm::vec2 uv2 = {tile.xend, tile.yend};
    gui::texture(text, uv, uv2);
    ImGui::End();
  }
}
