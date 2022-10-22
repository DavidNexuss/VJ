#include "application.hpp"
#include "entity/shot.hpp"
#include "entity/turret.hpp"
#include "ext/util.hpp"
#include "imgui.h"
#include "parallax.hpp"
#include "shambhala.hpp"
#include "ship.hpp"
#include "tile.hpp"
#include <ext.hpp>

using namespace shambhala;

struct ComponentSystem {
  ShotComponent *shotComponent;

  ComponentSystem() {
    shotComponent = new ShotComponent;
    addModel(shotComponent);
    addComponent(shotComponent);
    test();
  }

  void test() {
    int n = 2000;
    for (int i = 0; i < n; i++) {
      float p = i / float(n);
      float x = glm::cos(p * M_PI * 2);
      float y = glm::sin(p * M_PI * 2);
      shotComponent->addShot(glm::vec2(-3.0, 10.0), glm::vec2(x, y), 1);
    }
  }

  void registerEntity(Entity *ent) { shotComponent->addEntity(ent); }
};

ComponentSystem *comp;

void setupComponentSystem() { comp = new ComponentSystem; }

void setupShip() {
  Texture *baseColorShip = shambhala::createTexture();
  {
    baseColorShip->useNeareast = true;
    baseColorShip->clamp = true;
    baseColorShip->addTextureResource(
        resource::stbiTextureFile("textures/ship.png", 4));
  }

  DynamicPartAtlas *dyn = new DynamicPartAtlas();
  {
    dyn->textureAtlas = baseColorShip;
    dyn->renderingProgram =
        loader::loadProgram("programs/dynamic_tiled.fs", "programs/regular.vs");
    dyn->coords = joc_data_ship;
  }

  shambhala::Node *rootNode = shambhala::createNode();
  rootNode->setTransformMatrix(util::translate(0.0, 0.0, -0.1));

  Turret *turret = new Turret(dyn);
  turret->target(shambhala::createNode());
  shambhala::addComponent(dyn);
  shambhala::addComponent(turret);
}
void setupBackground() {
  Material *mat;

  ParallaxBackground *background = new ParallaxBackground;
  {
    mat = background->addParallaxBackground(
        "materials/layer3.mat", 1.0,
        loader::loadTexture("textures/cethiel_layer3.png", 4), -2);
    mat->set("zspeed", 0.001f);
    mat->set("offset", glm::vec2(0.0, 0.3));
    mat = background->addParallaxBackground(
        "materials/layer2.mat", 1.0,
        loader::loadTexture("textures/cethiel_layer2.png", 4), -4);
    mat->set("zspeed", 0.0005f);
    mat->set("offset", glm::vec2(0.0, 0.25));
    mat = background->addParallaxBackground(
        "materials/layer1.mat", 1.0,
        loader::loadTexture("textures/cethiel_layer1.png", 4), -5);
    mat->set("offset", glm::vec2(0.0, 0.25));
  }

  // Adding fog
  {
    Program *fog =
        loader::loadProgram("programs/fog.fs", "programs/parallax.vs");
    mat = background->addParallaxBackground(
        "materials/fog.mat", 1.0, loader::loadTexture("textures/fog.png", 4), 3,
        fog);
    mat->set("scale", glm::vec2(0.3, 1.0));
    mat->set("zspeed", 0.0015f);
  }

  shambhala::addComponent(background);
}

void setupLevel() {

  Texture *baseColor = shambhala::createTexture();
  baseColor->useNeareast = true;
  baseColor->clamp = true;
  baseColor->addTextureResource(
      resource::stbiTextureFile("textures/green_tile.png", 4));

  int sizex = 200;
  int sizey = 20;

  TileMap *tiles = new TileMap(sizex, sizey, new StaticTileAtlas, baseColor);
  tiles->loadLevel(resource::ioMemoryFile("levels/level01.txt"));

  shambhala::addComponent(tiles);
  comp->registerEntity(tiles);

  tiles = new TileMap(sizex, sizey, new StaticTileAtlas, baseColor);
  tiles->loadLevel(resource::ioMemoryFile("levels/level02.txt"));
  tiles->zindex = 9;
  shambhala::addComponent(tiles);
  comp->registerEntity(tiles);

  // Setup floor
  {
    Model *model = shambhala::createModel();
    model->node = shambhala::createNode();
    model->node->setName("floor");
    model->mesh = util::createTexturedQuad();
    model->material = shambhala::createMaterial();
    model->material->set("uv_scale", glm::vec2(100.0, 100.0));
    model->material->set("uv_offset", glm::vec2(1.0, 0.0));
    glm::mat4 transform;
    transform[0] = glm::vec4(0.0, 0.0, 1000.0, 0.0);
    transform[1] = glm::vec4(1000.0, 0.0, 0.0, 0.0);
    transform[2] = glm::vec4(0.0, 1.0, 0.0, 0.0);
    transform[3] = glm::vec4(-500.0, 0.0, -500.0, 1.0);
    model->zIndex = -3;
    model->node->setTransformMatrix(transform);
    model->program =
        loader::loadProgram("programs/floor.fs", "programs/regular.vs");
    {
      Texture *floorTexture = shambhala::createTexture();
      floorTexture->useNeareast = true;
      floorTexture->clamp = false;
      floorTexture->addTextureResource(
          resource::stbiTextureFile("textures/floor.png", 3));

      DynamicTexture dyn;
      dyn.unit = 5;
      dyn.sourceTexture = floorTexture;
      model->material->set("input", dyn);
    }
    {
      Texture *floorTexture = shambhala::createTexture();
      floorTexture->useNeareast = true;
      floorTexture->clamp = false;
      floorTexture->addTextureResource(
          resource::stbiTextureFile("textures/floor2.png", 3));

      DynamicTexture dyn;
      dyn.unit = 6;
      dyn.sourceTexture = floorTexture;
      model->material->set("input2", dyn);
    }
    addModel(model);
  }
}

struct ResWorldMat : public Material, LogicComponent {

  ResWorldMat() { hint_is_material = this; }
  void step(StepInfo info) override {
    set("uAR", float(viewport()->aspectRatio()));
  }
};
void setupBasic() {

  worldmats::Clock *clock = new worldmats::Clock;
  clock->setName("Clock");
  shambhala::setWorldMaterial(2, clock);
  shambhala::addComponent(clock);

  ResWorldMat *worldMat = new ResWorldMat;
  shambhala::setWorldMaterial(11, worldMat);
  shambhala::addComponent(worldMat);
}
void loadTestScene() {

  setupComponentSystem();
  setupLevel();
  setupBackground();
  setupShip();
  setupBasic();
}
int main() {
  Joc joc;
  joc.enginecreate();
  loadTestScene();
  joc.loop();
}
