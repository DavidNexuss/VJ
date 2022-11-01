#include "tile.hpp"
#include "ext/math.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
#include <cstdio>
#include <glm/ext/matrix_clip_space.hpp>
using namespace shambhala;

struct TileAttribute {
  glm::vec3 position;
  glm::vec2 uv;
};

TileMap::TileMap(int sizex, int sizey, TileAtlas *atlas, Texture *text,
                 int zindex) {
  tiles.resize(sizex * sizey);
  this->sizex = sizex;
  this->sizey = sizey;
  this->atlas = atlas;
  this->textureAtlas = text;
  this->zindex = zindex;

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
    model->material = shambhala::createMaterial();
    model->material->set("input", text);
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
    illuminationModel->zIndex = zindex;
  }

  enableBake(true);
  setName("tileMap");
  addModel(model);
  addModel(bakedModel);
  addModel(illuminationModel);
  // Create framebuffers
  {
    static auto createBakeFramebuffer = []() {
      auto *fbo = shambhala::createFramebuffer();
      fbo->addOutput({GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE});
      return fbo;
    };

    fbo_bake = createBakeFramebuffer();
    fbo_shadows = createBakeFramebuffer();

    fbo_shadows->clearColor.w = 1.0;

    bakedModel->material->set("input", fbo_bake->getOutputTexture(0));

    illuminationModel->material->set("input", fbo_bake->getOutputTexture(0));
    illuminationModel->material->set("shadow",
                                     fbo_shadows->getOutputTexture(0));
  }
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
  model->mesh->vbo->signalUpdate();
  model->mesh->ebo->signalUpdate();
  needsUpdate = false;

  bake();
  bakeShadows();
}

void TileMap::bakeShadows() {

  bakeCount++;
  static worldmats::SimpleCamera *camera = new worldmats::SimpleCamera;
  static Program *shadowGenerator = loader::loadProgram(
      "programs/tiled_shadow_generator.fs", "programs/regular.vs");

  glm::vec2 size = glm::vec2(sizex, sizey) * 16.0f;
  {
    fbo_shadows->begin(size.x, size.y);
    shadowGenerator->use();
    shadowGenerator->bind(bakedModel->material);
    camera->setProjectionMatrix(
        glm::ortho(0.0f, float(sizex), 0.0f, float(sizey)));
    camera->set(Standard::uTransformMatrix, util::scale(sizex, sizey, 1.0));
    bakedModel->mesh->use();

    glm::mat4 viewMat(1.0f);
    for (int i = 0; i < 200; i++) {
      camera->setViewMatrix(viewMat);
      viewMat = util::translate(-0.3 * 0.3, -0.6 * 0.3, 0.0) * viewMat;

      shadowGenerator->bind(camera);
      shadowGenerator->bind("shadowLevel", Uniform(float(i)));

      drawCall();
    }
    fbo_shadows->end();
  }
  illuminationModel->node->setTransformMatrix(util::scale(sizex, sizey, 1.001));
}
void TileMap::bake() {
  static worldmats::SimpleCamera *camera = new worldmats::SimpleCamera;
  camera->set(Standard::uTransformMatrix, glm::mat4(1.0));

  glm::vec2 size = glm::vec2(sizex, sizey) * 16.0f;

  glDisable(GL_BLEND);
  {
    fbo_bake->begin(size.x, size.y);
    {
      camera->setViewMatrix(glm::mat4(1.0f));
      camera->setProjectionMatrix(
          glm::ortho(0.0f, float(sizex), 0.0f, float(sizey)));

      model->program->use();
      model->mesh->use();
      model->program->bind(camera);
      model->program->bind(this->model->material);

      drawCall();
    }
    fbo_bake->end();
  }
  glEnable(GL_BLEND);

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
int TileMap::get(int i, int j) {
  if (i < 0 || j < 0 || i >= sizex || j >= sizey)
    return 0;
  return tiles[i + j * sizex];
}
#include <sstream>
void TileMap::initmap(IResource *resource) {
  io_buffer *buff = resource->read();
  std::stringstream ss(std::string((const char *)buff->data));
  std::string trash;
  getline(ss, trash); // READ MARKUP
  int trash1, trash2;
  ss >> trash1 >> trash2;
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

    levelResource.cleanFile()->set(io_buffer::create(serialize()));
    levelResource.signalAck();
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
  for (int i = sizey - 1; i >= 0; i--) {
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
  this->levelResourceEditor.acquire(leveldata);
}

Collision TileMap::inside(glm::vec2 position) {

  if (get(position.x, position.y) != 0) {
    Collision col;
    col.typeClass = COLLISION_WORLD;
    return col;
  }
  return Collision{};
}

void TileMap::signalHit(Collision col) {}

Collision TileMap::inside(AABB box) {
  glm::vec2 lowerCorner = box.lower;
  glm::vec2 highCorner = box.higher;
  int i = lowerCorner.x;
  int j = lowerCorner.y;

  int ii = highCorner.x;
  int jj = highCorner.y;

  // TODO get real corners x and y
  for (int x = i; x < ii; x++) {
    for (int y = j; y < jj; y++) {
      if (x >= 0 && y >= 0 && x < sizex && y < sizey) {
        if (inside(glm::vec2(x, y))) {
          Collision col;
          col.typeClass = COLLISION_WORLD;
          col.shortestPosition = glm::vec2(x, y);
          return col;
        }
      }
    }
  }
  return Collision{};
}
