#include "ext/util.hpp"
#include <ext.hpp>
#include <shambhala.hpp>

#include <impl/io_linux.hpp>
#include <impl/logger.hpp>
#include <impl/viewport_glfw.hpp>

using namespace shambhala;

struct Tile {
  float xstart;
  float ystart;
  float xend;
  float yend;
};

struct TileAtlas {
  std::unordered_map<int, Tile> tiles;
  Tile getTile(int tileindex) {
    Tile tile;
    float p = 16.0f / 512.0f;
    tile.xstart = (tileindex % 32) * p;
    tile.yend = (float(tileindex) / 32) * p;
    tile.xend = tile.xstart + p;
    tile.yend = tile.ystart + p;
    std::swap(tile.yend, tile.ystart);
    return tile;
  }
};

struct TileAttribute {
  glm::vec3 position;
  glm::vec2 uv;
};

struct TileMap {
  TileAtlas *atlas;
  Model *model;
  simple_vector<int> tiles;
  int tileMapSize;

  TileMap(int tileMapSize, TileAtlas *atlas, Texture *text) {
    tiles.resize(tileMapSize * tileMapSize);
    this->tileMapSize = tileMapSize;
    this->atlas = atlas;

    model = shambhala::createModel();
    model->mesh = shambhala::createMesh();
    model->mesh->vbo = shambhala::createVertexBuffer();
    model->mesh->ebo = shambhala::createIndexBuffer();
    model->mesh->vbo->attributes = {{Standard::aPosition, 3},
                                    {Standard::aUV, 2}};
    model->node = shambhala::createNode("tilemap");
    model->program =
        loader::loadProgram("programs/tiled.fs", "programs/regular.vs");
    model->material = shambhala::createMaterial();
    DynamicTexture dyn;
    dyn.sourceTexture = text;
    dyn.unit = 0;
    model->material->set("uBaseColor", dyn);
  }

  void updateMesh() {
    simple_vector<TileAttribute> tiles_buffer =
        model->mesh->vbo->vertexBuffer.drop();
    simple_vector<int> tile_indices = model->mesh->ebo->indexBuffer.drop();

    tiles_buffer.resize(4 * tileMapSize * tileMapSize);
    int count = 0;
    for (int i = 0; i < tiles.size(); i++) {
      int x = i % tileMapSize;
      int y = i / tileMapSize;
      if (tiles[i]) {
        Tile tile = atlas->getTile(tiles[i]);
        tiles_buffer[count] = {glm::vec3(x, y, 0.0),
                               glm::vec2(tile.xstart, tile.ystart)};

        tiles_buffer[count + 1] = {glm::vec3(x + 1, y, 0.0),
                                   glm::vec2(tile.xend, tile.ystart)};

        tiles_buffer[count + 2] = {glm::vec3(x, y + 1, 0.0),
                                   glm::vec2(tile.xstart, tile.yend)};

        tiles_buffer[count + 3] = {glm::vec3(x + 1, y + 1, 0.0),
                                   glm::vec2(tile.xend, tile.yend)};

        tile_indices.push(count);
        tile_indices.push(count + 1);
        tile_indices.push(count + 2);

        tile_indices.push(count + 3);
        tile_indices.push(count + 2);
        tile_indices.push(count + 1);

        count += 4;
      };
    }
    tiles_buffer.resize(count);
    model->mesh->vbo->vertexBuffer = tiles_buffer.drop();
    model->mesh->ebo->indexBuffer = tile_indices.drop();
    model->mesh->vbo->updateData = true;
    model->mesh->ebo->updateData = true;
  }
};

void enginecreate() {

  EngineParameters parameters;
  parameters.io = new shambhala::LinuxIO;
  parameters.viewport = new shambhala::ViewportGLFW;
  parameters.logger = new shambhala::DefaultLogger;
  parameters.io->translators.push_back("assets/%s");
  parameters.io->translators.push_back("internal_assets/%s");
  parameters.io->translators.push_back("machine/%s");
  parameters.io->translators.push_back("joc2d/%s");
  shambhala::createEngine(parameters);

  WindowConfiguration configuration;
  configuration.titlename = "Test main";
  configuration.width = 800;
  configuration.height = 600;
  configuration.mssaLevel = 4;
  configuration.openglMajorVersion = 4;
  configuration.openglMinorVersion = 3;

  shambhala::setActiveWindow(shambhala::createWindow(configuration));
}

void test(ModelList *scene) {
  Texture *baseColor = shambhala::createTexture();
  baseColor->useNeareast = true;
  baseColor->clamp = true;
  baseColor->addTextureResource(
      resource::stbiTextureFile("textures/green_tile.png", 4));

  TileMap *tiles = new TileMap(32, new TileAtlas, baseColor);
  tiles->tiles[0] = 1;
  tiles->tiles[2] = 1;
  tiles->tiles[1] = 1;
  tiles->tiles[5] = 1;
  tiles->updateMesh();
  scene->add(tiles->model);
}

void runEditor() {
  RenderCamera *camera = shambhala::createRenderCamera();
  ModelList *scene = shambhala::createModelList();
  RenderShot shot;
  shot.scenes.push(scene);
  shambhala::setWorkingModelList(scene);

  worldmats::Camera *cam = new worldmats::DebugCamera;
  shambhala::setWorldMaterial(Standard::wCamera, cam);
  test(scene);
  do {

    shambhala::loop_beginRenderContext();
    camera->render(shot);
    shambhala::loop_endRenderContext();
    shot.updateFrame();

  } while (!shambhala::loop_shouldClose());
}

int main() {
  enginecreate();
  runEditor();
}
