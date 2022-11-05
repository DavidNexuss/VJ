#include "application.hpp"
#include "entity/enemies/base.hpp"
#include "entity/enemies/turret.hpp"
#include "entity/player.hpp"
#include "entity/shot.hpp"
#include "ext/util.hpp"
#include "imgui.h"
#include "parallax.hpp"
#include "playerCamera.hpp"
#include "shambhala.hpp"
#include "ship.hpp"
#include "tile.hpp"
#include <ext.hpp>
#include <fstream>

using namespace shambhala;

static int playerCoords[] = {20, 14, 43, 34};

struct ComponentSystem {
  PlayerCamera *camera = nullptr;
  ShotComponent *shotComponent = nullptr;
  Player *playerComponent = nullptr;
  simple_vector<EntityComponent *> components;

  ComponentSystem() {
    initShotComponent();
    initPlayer();
    initEnemies();
#ifndef DEBUGCAMERA
    initCamera();
#endif
  }

  void initCamera() {

    PlayerCamera *cam = new PlayerCamera("joc2d/data/cam.txt",
                                         playerComponent->getPlayerPosition());
    shambhala::pushMaterial(cam);
    shambhala::addComponent(cam);
  }

  void initShotComponent() {

    shotComponent = new ShotComponent;
    addModel(shotComponent);
    addComponent(shotComponent);
  }
  void initPlayer() {

    DynamicPartAtlas *dyn = new DynamicPartAtlas;
    dyn->textureAtlas = loader::loadTexture("textures/player.png", 4);
    dyn->textureAtlas->useNeareast = true;
    dyn->coords = playerCoords;
    dyn->renderingProgram =
        loader::loadProgram("programs/dynamic_tiled.fs", "programs/regular.vs");

    playerComponent = new Player(shotComponent, dyn);
    playerComponent->setPosition({8, 11});
    playerComponent->setName("Player");
    addComponent(playerComponent);
  }

  void spawnEnemies(const char *source, BaseEnemy *b) {
    std::ifstream file(source);

    int clas;
    float x;
    float y;
    while (file >> clas >> x >> y) {
      b->spawnEnemy(clas, x, y);
    }
  }
  void initEnemies() {

    BaseEnemy *guy = new BaseEnemy(shotComponent);
    guy->target = playerComponent->getPlayerPosition();
    guy->setName("Guy");
    addComponent(guy);
    components.push(guy);
    {

      // Create guy class
      {
        static int guyCoords[] = {0,   32, 36,  32, 72, 32, 36,  32, 108, 32,
                                  36,  32, 144, 32, 36, 32, 180, 32, 36,  32,
                                  216, 32, 36,  32,

                                  0,   64, 36,  32, 72, 64, 36,  32, 108, 64,
                                  36,  32, 144, 64, 36, 32, 180, 64, 36,  32};
        EnemyClass guyClass;
        guyClass.atlas =
            createEnemyAtlas("textures/grenade_guy.png", guyCoords);
        guyClass.scaleTransform = util::scale(2.0);
        guyClass.regularAnimationCount = 6;
        guyClass.attackAnimationCount = 3;
        guyClass.shotCenter = glm::vec2(0.2, 0.9);
        guyClass.shot = true;
        guyClass.fly = false;

        guy->createEnemyClass(0, guyClass);
      }

      // Create turret class
      {

      }

      // Create jumper class
      {

      }

      // Create shooter
      {}

      guy->spawnEnemy(0, 80, 11);
      guy->spawnEnemy(0, 70, 11);
      guy->spawnEnemy(0, 73, 15);

      spawnEnemies("joc2d/data/enemies.txt", guy);
    }
  }

  void registerEntity(Entity *ent) {
    shotComponent->addEntity(ent);
    playerComponent->addEntity(ent);
    for (int i = 0; i < components.size(); i++) {
      components[i]->addEntity(ent);
    }
  }

private:
  DynamicPartAtlas *createEnemyAtlas(const char *texturePath, int *cords) {

    Texture *text = loader::loadTexture(texturePath, 4);
    text->useNeareast = true;
    shambhala::Program *program =
        loader::loadProgram("programs/dynamic_tiled.fs", "programs/regular.vs");
    DynamicPartAtlas *atlas = DynamicPartAtlas::create(program, text, cords);
    return atlas;
  }
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

  shambhala::addComponent(dyn);
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

TileMap *createLayer(const char *filename, int sizex, int sizey, int zindex,
                     Texture *texture, float z) {
  TileMap *tiles =
      new TileMap(sizex, sizey, new StaticTileAtlas, texture, zindex);
  tiles->loadLevel(resource::ioMemoryFile(filename));
  tiles->rootNode->transform(util::translate(0.0, 0.0, z));

  addComponent(tiles);
  return tiles;
}
void setupLevel() {

  Texture *baseColor = shambhala::createTexture();
  baseColor->useNeareast = true;
  baseColor->clamp = true;
  baseColor->addTextureResource(
      resource::stbiTextureFile("textures/green_tile.png", 4));

  int sizex = 200;
  int sizey = 20;

  TileMap *map =
      createLayer("levels/level01.txt", sizex, sizey, 1, baseColor, 0.0);
  comp->registerEntity(map);
  map->setName("MainLevel");

  map =
      createLayer("levels/level01_bak.txt", sizex, sizey, 0, baseColor, -0.01);
  map->setName("MainLevelBackground");

  map = createLayer("levels/level01_1.txt", sizex, sizey, -1, baseColor, -5.0);
  map->setName("Level Back");

  map = createLayer("levels/level01_2.txt", sizex, sizey, 11, baseColor, 5.0);
  map->setName("Level Front");

  map = createLayer("levels/Level1-3.txt", sizex, sizey, 1, baseColor, 0.0);
  map->setName("Level 2 Main");
  map->rootNode->setOffset(glm::vec3(200, 0, 0));
  comp->registerEntity(map);

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

      model->material->set("input", floorTexture);
    }
    {
      Texture *floorTexture = shambhala::createTexture();
      floorTexture->useNeareast = true;
      floorTexture->clamp = false;
      floorTexture->addTextureResource(
          resource::stbiTextureFile("textures/floor2.png", 3));

      model->material->set("input2", floorTexture);
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
  shambhala::pushMaterial(clock);
  shambhala::addComponent(clock);

  ResWorldMat *worldMat = new ResWorldMat;
  shambhala::pushMaterial(worldMat);
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
