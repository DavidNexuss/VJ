
#include "shambhala.hpp"
#include "tile.hpp"
#include <ext.hpp>
#include <string>
using namespace shambhala;

#include <imgui/imgui.h>

//-----------------------------[TILE MAP]-----------------------------------

int tilehit = 0;
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

  glm::vec3 intersection = ext::rayIntersection(info.mouseRay, plane);
  int x = intersection.x;
  int y = intersection.y;

  if (x >= 0 && y >= 0 && x < sizex && y < sizey) {

    tilehit = get(x, y);

    if (viewport()->isRightMousePressed()) {
      set(x, y, this->editorMaterial);
    }

    if (viewport()->isKeyPressed(KEY_X)) {
      set(x, y, 0);
    }
  }

  if (viewport()->isKeyJustPressed(KEY_UP)) {
    this->editorMaterial -= 32;
  }

  if (viewport()->isKeyJustPressed(KEY_DOWN)) {
    this->editorMaterial += 32;
  }
  if (viewport()->isKeyJustPressed(KEY_RIGHT)) {
    this->editorMaterial++;
  }

  if (viewport()->isKeyJustPressed(KEY_LEFT)) {
    this->editorMaterial--;
  }

  this->editorMaterial = this->editorMaterial % (32 * 32);
}
void TileMap::editorRender() {
  {
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
  }

  if (ImGui::Begin("Tile")) {
    ImGui::InputInt("Current: ", &this->editorMaterial);
    ImGui::Text("Bake count %d", bakeCount);
    ImGui::Text("Tile hit %d", int(tilehit));
    Tile tile = atlas->getTile(this->editorMaterial);
    glm::vec2 uv = {tile.xstart, tile.ystart};
    glm::vec2 uv2 = {tile.xend, tile.yend};
    gui::texture(textureAtlas, uv, uv2, glm::vec2(64.0));
  }

  if (ImGui::Begin("TileMap")) {
    gui::texture(textureAtlas, glm::vec2(0.0, 1.0), glm::vec2(1.0, 0.0),
                 glm::vec2(512.0));
  }
  /*
  if (bakeInformation.bakedTexture >= 0 && ImGui::Begin("Baked world")) {
    gui::texture(bakeInformation.bakedTexture, glm::vec2(0.0, 1.0),
                 glm::vec2(1.0, 0.0), bakeInformation.size);
  }*/

  if (this->levelResource.cleanFile())
    gui::textEditor(this->levelResourceEditor, "LevelResource");
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

#include "entity/entity.hpp"
void DynamicPartAtlas::editorRender() {
  if (ImGui::Begin("DynamicPart")) {

    for (int i = 0; i < 100; i++) {
      int part = i;

      glm::vec2 textureSize =
          glm::vec2(textureAtlas->textureData[0].cleanFile()->width,
                    textureAtlas->textureData[0].cleanFile()->height);

      glm::vec2 offset =
          glm::vec2(coords[part * 4], coords[part * 4 + 1]) / textureSize;
      glm::vec2 scale = glm::vec2(coords[part * 4 + 2], coords[part * 4 + 3]);

      glm::vec2 renderScale = scale;
      scale /= textureSize;

      glm::vec2 uv1 = offset;
      glm::vec2 uv2 = offset + scale;

      gui::texture(textureAtlas, uv1, uv2, renderScale);

      ImGui::InputInt("Part index", &editor_part);
      ImGui::Text("%d\n", part);
    }
    ImGui::End();
  }
}
