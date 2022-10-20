#include "tile.hpp"
#include "ext/math.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
#include <cstdio>
#include <glm/ext/matrix_clip_space.hpp>
using namespace shambhala;

Tile StaticTile::getTile(int tileindex) {
  Tile tile;
  float p = 16.0f / 512.0f;
  tile.xstart = (tileindex % 32) * p;
  tile.ystart = float(tileindex / 32) * p;
  tile.xend = tile.xstart + p;
  tile.yend = tile.ystart + p;
  std::swap(tile.yend, tile.ystart);
  return tile;
}

TileMap::TileMap(int sizex, int sizey, TileAtlas *atlas, Texture *text) {
  tiles.resize(sizex * sizey);
  this->sizex = sizex;
  this->sizey = sizey;
  this->atlas = atlas;
  this->textureAtlas = text;

  Program *renderProgram =
      loader::loadProgram("programs/tiled.fs", "programs/regular.vs");

  // Create regular model
  {
    model = shambhala::createModel();

    // Create mesh
    {
      model->mesh = shambhala::createMesh();
      model->mesh->vbo = shambhala::createVertexBuffer();
      model->mesh->ebo = shambhala::createIndexBuffer();
      model->mesh->vbo->attributes = {{Standard::aPosition, 3},
                                      {Standard::aUV, 2}};
    }

    model->program = renderProgram;
    DynamicTexture dyn;
    dyn.sourceTexture = text;
    dyn.unit = 0;
    model->material = shambhala::createMaterial();
    model->material->set("input", dyn);
    model->node = shambhala::createNode("tileMap");
  }

  // Create baked model
  {
    bakedModel = shambhala::createModel();
    bakedModel->program =
        loader::loadProgram("programs/parallax.fs", "programs/regular.vs");
    bakedModel->mesh = util::createTexturedQuad();
    bakedModel->material = shambhala::createMaterial();
    bakedModel->node = shambhala::createNode("tileMapBacked");
  }

  enableBake(true);
  // setDebug(model);
  setName("tileMap");
  addModel(model);
  addModel(bakedModel);
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
  simple_vector<TileAttribute> vertices;
  simple_vector<int> indices;

  indices.resize(0);
  vertices.resize(4 * sizex * sizey);

  int count = 0;
  for (int i = 0; i < tiles.size(); i++) {
    int x = i % sizex;
    int y = i / sizex;
    if (tiles[i]) {
      Tile tile = atlas->getTile(tiles[i]);
      vertices[count] = {glm::vec3(x, y, 0.0),
                         glm::vec2(tile.xstart, tile.ystart)};

      vertices[count + 1] = {glm::vec3(x + 1, y, 0.0),
                             glm::vec2(tile.xend, tile.ystart)};

      vertices[count + 2] = {glm::vec3(x, y + 1, 0.0),
                             glm::vec2(tile.xstart, tile.yend)};

      vertices[count + 3] = {glm::vec3(x + 1, y + 1, 0.0),
                             glm::vec2(tile.xend, tile.yend)};

      indices.push(count);
      indices.push(count + 1);
      indices.push(count + 2);

      indices.push(count + 3);
      indices.push(count + 2);
      indices.push(count + 1);
      count += 4;
    };
  }
  vertices.resize(count);
  model->mesh->vbo->vertexBuffer = vertices.drop();
  model->mesh->ebo->indexBuffer = indices.drop();
  model->mesh->vbo->updateData = true;
  model->mesh->ebo->updateData = true;
  needsUpdate = false;

  bake();
}

void TileMap::bake() {
  static FrameBuffer *fbo = nullptr;
  static worldmats::SimpleCamera *camera = new worldmats::SimpleCamera;

  if (fbo == nullptr) {
    fbo = shambhala::createFramebuffer();
    FrameBufferAttachmentDescriptor desc;
    desc.externalFormat = GL_RGBA;
    desc.internalFormat = GL_RGBA;
    desc.type = GL_UNSIGNED_BYTE;
    desc.useNeareast = true;
    fbo->addChannel(desc);
  }

  TileBake bakeResult;
  bakeResult.size = glm::vec2(sizex, sizey) * 16.0f;

  shambhala::engine_clearState();
  {
    viewport()->fakeViewportSize(bakeResult.size.x, bakeResult.size.y);
    shambhala::updateViewport();
    fbo->begin(bakeResult.size.x, bakeResult.size.y);
    {
      camera->setViewMatrix(glm::mat4(1.0f));
      camera->setProjectionMatrix(
          glm::ortho(0.0f, float(sizex), 0.0f, float(sizey)));

      device::useProgram(this->model->program);
      device::useMesh(this->model->mesh);
      device::useMaterial(this->model->material);
      device::useMaterial(camera);
      device::useMaterial(this->model->node);
      device::drawCall();
    }
    fbo->end();
    viewport()->restoreViewport();
    shambhala::updateViewport();
  }
  shambhala::engine_clearState();

  bakeResult.bakedTexture = fbo->colorAttachments[0];
  this->bakeInformation = bakeResult;

  UTexture texture;
  texture.mode = GL_TEXTURE_2D;
  texture.texID = bakeInformation.bakedTexture;
  texture.unit = 0;
  bakedModel->material->set("input", texture);
  bakedModel->node->setTransformMatrix(util::scale(sizex, sizey, 1.001));
}
void TileMap::enableBake(bool pEnable) {
  model->node->setEnabled(!pEnable);
  bakedModel->node->setEnabled(pEnable);
}
void TileMap::set(int i, int j, int type) {
  tiles[i + j * sizex] = type;
  needsUpdate = true;
}
#include <sstream>
void TileMap::initmap(IResource *resource) {
  io_buffer *buff = resource->read();
  std::stringstream ss(std::string((const char *)buff->data));
  std::string trash;
  getline(ss, trash); // READ MARKUP
  ss >> this->sizex >> this->sizey;
  getline(ss, trash); // READ SIZE
  getline(ss, trash); // READ TILE SIZE
  getline(ss, trash); // READ TILENAME
  getline(ss, trash); // READ TILES IN TILESHEET
  tiles.resize(sizex * sizey);
  for (int i = sizey - 1; i >= 0; i--) {
    std::string meshline;
    getline(ss, meshline);
    std::stringstream ss2(meshline);
    for (int j = 0; j < sizex; j++) {
      ss2 >> tiles[i * sizex + j];
    }
  }
  resource->needsUpdate = false;
}
void TileMap::step(shambhala::StepInfo info) {

  if (levelResource && levelResource->needsUpdate) {
    initmap(levelResource);
    needsUpdate = true;
  }
  if (needsUpdate)
    updateMesh();
}

void TileMap::serialize() {
  printf("%d %d\n", sizex, sizey);
  for (int i = 0; i < tiles.size(); i++) {
    printf("%d ", tiles[i]);
  }
}

void TileMap::loadLevel(IResource *leveldata) {
  this->levelResource = leveldata;
}
