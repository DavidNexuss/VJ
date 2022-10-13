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

void setDebug(Model *model) {

  model->program = util::createBasicColored();
  model->material = shambhala::createMaterial();
  model->material->set("uColor", glm::vec4(1.0));
  model->polygonMode = GL_LINE;
}
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
    DynamicTexture dyn;
    dyn.sourceTexture = text;
    dyn.unit = 0;
    model->material = shambhala::createMaterial();
    model->material->set("uBaseColor", dyn);

    // setDebug(model);
  }

  int spawnFace(simple_vector<TileAttribute> &vertexBuffer,
                simple_vector<int> &indexBuffer, int count, Tile tile, float x,
                float y, float p, int orientation) {

    float z = float(orientation % 2) * p;
    if (orientation == 0 || orientation == 1) {
      vertexBuffer[count] = {glm::vec3(x, y, z),
                             glm::vec2(tile.xstart, tile.ystart)};

      vertexBuffer[count + 1] = {glm::vec3(x + p, y, z),
                                 glm::vec2(tile.xend, tile.ystart)};

      vertexBuffer[count + 2] = {glm::vec3(x, y + p, z),
                                 glm::vec2(tile.xstart, tile.yend)};

      vertexBuffer[count + 3] = {glm::vec3(x + p, y + p, z),
                                 glm::vec2(tile.xend, tile.yend)};
    } else if (orientation == 2 || orientation == 3) {

      vertexBuffer[count] = {glm::vec3(x + z, y, 0.0),
                             glm::vec2(tile.xstart, tile.ystart)};

      vertexBuffer[count + 1] = {glm::vec3(x + z, y, 1.0),
                                 glm::vec2(tile.xend, tile.ystart)};

      vertexBuffer[count + 2] = {glm::vec3(x + z, y + p, 0.0),
                                 glm::vec2(tile.xstart, tile.yend)};

      vertexBuffer[count + 3] = {glm::vec3(x + z, y + p, 1.0),
                                 glm::vec2(tile.xend, tile.yend)};
    } else if (orientation == 4 || orientation == 5) {

      vertexBuffer[count] = {glm::vec3(x, y + z, 0.0),
                             glm::vec2(tile.xstart, tile.ystart)};

      vertexBuffer[count + 1] = {glm::vec3(x, y + z, 1.0),
                                 glm::vec2(tile.xend, tile.ystart)};

      vertexBuffer[count + 2] = {glm::vec3(x + p, y + z, 0.0),
                                 glm::vec2(tile.xstart, tile.yend)};

      vertexBuffer[count + 3] = {glm::vec3(x + p, y + z, 1.0),
                                 glm::vec2(tile.xend, tile.yend)};
    }
    bool swizz = orientation == 1 || orientation == 2 || orientation == 5;

    if (!swizz) {
      indexBuffer.push(count + 2);
      indexBuffer.push(count + 1);
      indexBuffer.push(count);

      indexBuffer.push(count + 1);
      indexBuffer.push(count + 2);
      indexBuffer.push(count + 3);
    } else {

      indexBuffer.push(count);
      indexBuffer.push(count + 1);
      indexBuffer.push(count + 2);

      indexBuffer.push(count + 3);
      indexBuffer.push(count + 2);
      indexBuffer.push(count + 1);
    }
    return count + 4;
  }

  int spawnTile(simple_vector<TileAttribute> &vertexBuffer,
                simple_vector<int> &indexBuffer, int count, Tile tile, int x,
                int y) {
    return spawnFace(vertexBuffer, indexBuffer, count, tile, x, y, 1.0, 1);
  }

  int spawnVoxel(simple_vector<TileAttribute> &vertexBuffer,
                 simple_vector<int> &indexBuffer, int count, Tile tile, int x,
                 int y) {
    static int l = 1;
    static float p = 1.0f / l;
    for (int i = 0; i < l; i++) {
      for (int j = 0; j < l; j++) {
        float xx = (x * l + i) / float(l);
        float yy = (y * l + j) / float(l);

        count = spawnFace(vertexBuffer, indexBuffer, count, tile, xx, yy, p, 0);
        count = spawnFace(vertexBuffer, indexBuffer, count, tile, xx, yy, p, 1);
        count = spawnFace(vertexBuffer, indexBuffer, count, tile, xx, yy, p, 2);
        count = spawnFace(vertexBuffer, indexBuffer, count, tile, xx, yy, p, 3);
        count = spawnFace(vertexBuffer, indexBuffer, count, tile, xx, yy, p, 4);
        count = spawnFace(vertexBuffer, indexBuffer, count, tile, xx, yy, p, 5);
      }
    }
    return count;
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

  tiles->tiles[1] = 2;
  tiles->tiles[2] = 2;
  tiles->tiles[3] = 2;
  tiles->tiles[4] = 2;
  tiles->tiles[5] = 3;
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
