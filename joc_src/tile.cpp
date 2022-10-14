#include "tile.hpp"
#include "shambhala.hpp"
using namespace shambhala;

Tile StaticTile::getTile(int tileindex) {
  Tile tile;
  float p = 16.0f / 512.0f;
  tile.xstart = (tileindex % 32) * p;
  tile.yend = (float(tileindex) / 32) * p;
  tile.xend = tile.xstart + p;
  tile.yend = tile.ystart + p;
  std::swap(tile.yend, tile.ystart);
  return tile;
}

TileMap::TileMap(int tileMapSize, TileAtlas *atlas, Texture *text) {
  tiles.resize(tileMapSize * tileMapSize);
  this->tileMapSize = tileMapSize;
  this->atlas = atlas;

  model = shambhala::createModel();
  model->mesh = shambhala::createMesh();
  model->mesh->vbo = shambhala::createVertexBuffer();
  model->mesh->ebo = shambhala::createIndexBuffer();
  model->mesh->vbo->attributes = {{Standard::aPosition, 3}, {Standard::aUV, 2}};
  model->node = shambhala::createNode("tilemap");
  model->program =
      loader::loadProgram("programs/tiled.fs", "programs/regular.vs");
  DynamicTexture dyn;
  dyn.sourceTexture = text;
  dyn.unit = 0;
  model->material = shambhala::createMaterial();
  model->material->set("uBaseColor", dyn);

  // setDebug(model);
  shambhala::addModel(model);
}

int TileMap::spawnFace(simple_vector<TileAttribute> &vertexBuffer,
                       simple_vector<int> &indexBuffer, int count, Tile tile,
                       float x, float y, float p, int orientation) {

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

int TileMap::spawnTile(simple_vector<TileAttribute> &vertexBuffer,
                       simple_vector<int> &indexBuffer, int count, Tile tile,
                       int x, int y) {
  return spawnFace(vertexBuffer, indexBuffer, count, tile, x, y, 1.0, 1);
}

int TileMap::spawnVoxel(simple_vector<TileAttribute> &vertexBuffer,
                        simple_vector<int> &indexBuffer, int count, Tile tile,
                        int x, int y) {
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
void TileMap::updateMesh() {
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
  needsUpdate = false;
}

void TileMap::set(int i, int j, int type) {
  tiles[i + j * tileMapSize] = type;
  needsUpdate = true;
}

void TileMap::step(shambhala::StepInfo info) {
  if (needsUpdate)
    updateMesh();
}
