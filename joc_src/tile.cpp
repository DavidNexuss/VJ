#include "tile.hpp"
#include "ext/math.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
#include <cstdio>
#include <glm/ext/matrix_clip_space.hpp>
using namespace shambhala;

TileMap::TileMap(int sizex, int sizey, TileAtlas *atlas, Texture *text) {
  tiles.resize(sizex * sizey);
  this->sizex = sizex;
  this->sizey = sizey;
  this->atlas = atlas;
  this->textureAtlas = text;

  Program *renderProgram =
      loader::loadProgram("programs/tiled.fs", "programs/regular.vs");

  rootNode = shambhala::createNode("Tile map root");
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
    model->node->setParentNode(rootNode);
  }

  // Create baked model
  {
    bakedModel = shambhala::createModel();
    bakedModel->program =
        loader::loadProgram("programs/parallax.fs", "programs/regular.vs");
    bakedModel->mesh = util::createTexturedQuad();
    bakedModel->material = shambhala::createMaterial();
    bakedModel->node = shambhala::createNode("tileMapBacked");
    bakedModel->node->setParentNode(rootNode);
  }

  // Create final result model
  {

    illuminationModel = shambhala::createModel();
    illuminationModel->program =
        loader::loadProgram("programs/tiled_shadows.fs", "programs/regular.vs");
    illuminationModel->mesh = util::createTexturedQuad();
    illuminationModel->material = shambhala::createMaterial();
    illuminationModel->node = shambhala::createNode("tileMapShadows");
    illuminationModel->node->setParentNode(rootNode);
  }

  enableBake(true);
  // setDebug(model);
  setName("tileMap");
  addModel(model);
  addModel(bakedModel);
  addModel(illuminationModel);
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
  bakeShadows();
}

void TileMap::bakeShadows() {

  bakeCount++;
  static worldmats::SimpleCamera *camera = new worldmats::SimpleCamera;
  static Program *shadowGenerator = loader::loadProgram(
      "programs/tiled_shadow_generator.fs", "programs/regular.vs");

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
    fbo->clearColor.w = 1.0;
    fbo->begin(bakeResult.size.x, bakeResult.size.y);
    glm::mat4 viewMat(1.0f);
    for (int i = 0; i < 200; i++) {
      camera->setViewMatrix(viewMat);
      viewMat = util::translate(-0.3 * 0.3, -0.6 * 0.3, 0.0) * viewMat;
      camera->setProjectionMatrix(
          glm::ortho(0.0f, float(sizex), 0.0f, float(sizey)));

      device::useProgram(shadowGenerator);
      device::useMesh(this->bakedModel->mesh);
      device::useMaterial(this->bakedModel->material);
      device::useMaterial(camera);
      device::useMaterial(this->bakedModel->node);

      device::useUniform("shadowLevel", Uniform(float(i)));
      device::drawCall();
    }
    fbo->end();
    viewport()->restoreViewport();
    shambhala::updateViewport();
  }
  shambhala::engine_clearState();
  bakeResult.bakedTexture = fbo->colorAttachments[0];
  this->bakeInformationShadow = bakeResult;

  UTexture texture;
  texture.mode = GL_TEXTURE_2D;
  texture.texID = bakeInformationShadow.bakedTexture;
  texture.unit = 1;
  UTexture base;
  base.mode = GL_TEXTURE_2D;
  base.texID = bakeInformation.bakedTexture;
  base.unit = 0;
  illuminationModel->material->set("input", base);
  illuminationModel->material->set("shadow", texture);
  illuminationModel->node->setTransformMatrix(util::scale(sizex, sizey, 1.001));
}
void TileMap::bake() {
  static worldmats::SimpleCamera *camera = new worldmats::SimpleCamera;

  if (bake_fbo == nullptr) {
    bake_fbo = shambhala::createFramebuffer();
    FrameBufferAttachmentDescriptor desc;
    desc.externalFormat = GL_RGBA;
    desc.internalFormat = GL_RGBA;
    desc.type = GL_UNSIGNED_BYTE;
    desc.useNeareast = true;
    bake_fbo->addChannel(desc);
  }

  TileBake bakeResult;
  bakeResult.size = glm::vec2(sizex, sizey) * 16.0f;

  shambhala::engine_clearState();

  glDisable(GL_BLEND);
  {
    viewport()->fakeViewportSize(bakeResult.size.x, bakeResult.size.y);
    shambhala::updateViewport();
    bake_fbo->begin(bakeResult.size.x, bakeResult.size.y);
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
    bake_fbo->end();
    viewport()->restoreViewport();
    shambhala::updateViewport();
  }
  shambhala::engine_clearState();

  glEnable(GL_BLEND);

  bakeResult.bakedTexture = bake_fbo->colorAttachments[0];
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
  bakedModel->node->setEnabled(false);
  illuminationModel->node->setEnabled(pEnable);
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
}
void TileMap::step(shambhala::StepInfo info) {

  if (levelResource.file()) {
    initmap(levelResource.file());
    levelResource.signalAck();
    needsUpdate = true;
  }
  if (needsUpdate) {
    updateMesh();
    // updateShadows();
  }
}

#include <ostream>
std::string TileMap::serialize() {
  std::ostringstream ss;
  ss << "TILEMAP\n";
  ss << sizex << " " << sizey << "\n";
  ss << "16 16\n";
  ss << "tile.png---dat \n";
  ss << "32 32\n";
  for (int i = 0; i < sizey; i++) {
    for (int j = 0; j < sizex; j++) {
      ss << tiles[i * sizex + j] << " ";
    }
    ss << "\n";
  }
  return ss.str();
}

void TileMap::save() { levelResource.cleanFile()->write(); }

void TileMap::loadLevel(IResource *leveldata) {
  this->levelResource.acquire(leveldata);
}

bool TileMap::inside(glm::vec2 position) {
  float x1 = glm::floor(position.x);
  float y1 = glm::floor(position.y);

  int x = x1;
  int y = y1;

  if (x < 0 || y < 0 || x >= sizex || y >= sizey)
    return 0;

  bool partialhit = tiles[x * sizex + y];
  return partialhit;
}

void TileMap::signalHit() {}
