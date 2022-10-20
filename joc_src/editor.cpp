
#include "shambhala.hpp"
#include "tile.hpp"
#include <ext.hpp>
#include <string>
using namespace shambhala;

#include <imgui/imgui.h>

//-----------------------------[TILE MAP]-----------------------------------
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

    if (viewport()->isKeyPressed(KEY_X)) {
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
    gui::texture(textureAtlas, uv, uv2, glm::vec2(64.0));
  }

  if (ImGui::Begin("TileMap")) {
    gui::texture(textureAtlas, glm::vec2(0.0, 1.0), glm::vec2(1.0, 0.0),
                 glm::vec2(512.0));
  }

  if (bakeInformation.bakedTexture >= 0 && ImGui::Begin("Baked world")) {
    gui::texture(bakeInformation.bakedTexture, glm::vec2(0.0, 1.0),
                 glm::vec2(1.0, 0.0), bakeInformation.size);
  }

  if (this->levelResource.cleanFile())
    gui::textEditor(this->levelResource.cleanFile(), "LevelResource");
}

#include "parallax.hpp"

void ParallaxBackground::editorRender() {
  simple_vector<std::string> names;
  for (int i = 0; i < models.size(); i++) {
    names.push("Parallax " + std::to_string(i));
  }
  static int selected = -1;
  if (ImGui::Begin("Parallax Maps")) {
    selected = gui::selectableList(names, selected);
    ImGui::End();
  }

  if (selected != -1) {
    if (ImGui::Begin("ParallaxMaterial")) {
      gui::materialEditor(models[selected]->material);
      ImGui::End();
    }
  }
}
