#include "application.hpp"
#include "entity/enemies/base.hpp"
#include "entity/enemies/finalboss.hpp"
#include "entity/enemies/turret.hpp"
#include "entity/forceShot.hpp"
#include "entity/player.hpp"
#include "entity/shot.hpp"
#include "ext/util.hpp"
#include "globals.hpp"
#include "imgui.h"
#include "parallax.hpp"
#include "playerCamera.hpp"
#include "shambhala.hpp"
#include "ship.hpp"
#include "tile.hpp"
#include <ext.hpp>
#include <fstream>
#include <iterator>

using namespace shambhala;

static int playerCoords[] = {20, 14, 43, 34};

struct ComponentSystem {
  PlayerCamera *camera = nullptr;
  FinalBoss *boss = nullptr;
  ShotComponent *shotComponent = nullptr;
  ForceShotComponent *forceShot = nullptr;
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

    forceShot = new ForceShotComponent;
    addComponent(forceShot);
  }
  void initPlayer() {

    DynamicPartAtlas *dyn = new DynamicPartAtlas;
    dyn->textureAtlas = loader::loadTexture("textures/player.png", 4);
    dyn->textureAtlas->useNeareast = true;
    dyn->coords = playerCoords;
    dyn->renderingProgram =
        loader::loadProgram("programs/dynamic_tiled.fs", "programs/regular.vs");

    playerComponent = new Player(shotComponent, forceShot, dyn);
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

    BaseEnemy *turret = new BaseEnemy(shotComponent);
    turret->target = playerComponent->getPlayerPosition();
    turret->setName("turret");
    addComponent(turret);
    components.push(turret);

    BaseEnemy *jumper = new BaseEnemy(shotComponent);
    jumper->target = playerComponent->getPlayerPosition();
    jumper->setName("jumper");
    addComponent(jumper);
    components.push(jumper);

    BaseEnemy *shooter = new BaseEnemy(shotComponent);
    shooter->target = playerComponent->getPlayerPosition();
    shooter->setName("shooter");
    addComponent(shooter);
    components.push(shooter);

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
        static int turretCoords[] = {
            0,   32, 36, 32, 72,  32, 36, 32, 108, 32, 36, 32,
            144, 32, 36, 32, 180, 32, 36, 32, 216, 32, 36, 32,

            0,   64, 36, 32, 72,  64, 36, 32, 108, 64, 36, 32,
            144, 64, 36, 32, 180, 64, 36, 32};
        EnemyClass turretClass;
        turretClass.atlas =
            createEnemyAtlas("textures/grenade_guy.png", turretCoords);
        turretClass.scaleTransform = util::scale(2.0);
        turretClass.regularAnimationCount = 6;
        turretClass.attackAnimationCount = 3;
        turretClass.shotCenter = glm::vec2(0.2, 0.9);
        turretClass.shot = true;
        turretClass.fly = false;

        turret->createEnemyClass(0, turretClass);
      }

      // Create jumper class
      {
        static int jumperCoords[] = {
            0,   32, 36, 32, 72,  32, 36, 32, 108, 32, 36, 32,
            144, 32, 36, 32, 180, 32, 36, 32, 216, 32, 36, 32,

            0,   64, 36, 32, 72,  64, 36, 32, 108, 64, 36, 32,
            144, 64, 36, 32, 180, 64, 36, 32};
        EnemyClass jumperClass;
        jumperClass.atlas =
            createEnemyAtlas("textures/grenade_guy.png", jumperCoords);
        jumperClass.scaleTransform = util::scale(2.0);
        jumperClass.regularAnimationCount = 6;
        jumperClass.attackAnimationCount = 3;
        jumperClass.shotCenter = glm::vec2(0.2, 0.9);
        jumperClass.shot = false;
        jumperClass.fly = false;
        jumperClass.jump = true;

        jumper->createEnemyClass(0, jumperClass);
      }

      // Create shooter
      {
        static int shooterCoords[] = {
            0,  32,  36,  32, 72, 32,  36,  32, 108, 32, 36,  32, 144, 32, 36,
            32, 180, 32,  36, 32, 216, 32,  36, 32,  0,  64,  36, 32,  72, 64,
            36, 32,  108, 64, 36, 32,  144, 64, 36,  32, 180, 64, 36,  32};
        EnemyClass shooterClass;
        shooterClass.atlas =
            createEnemyAtlas("textures/grenade_guy.png", shooterCoords);
        shooterClass.scaleTransform = util::scale(2.0);
        shooterClass.regularAnimationCount = 6;
        shooterClass.attackAnimationCount = 3;
        shooterClass.shotCenter = glm::vec2(0.2, 0.9);
        shooterClass.shot = true;
        shooterClass.fly = true;
        shooterClass.jump = true;

        shooter->createEnemyClass(0, shooterClass);
      }

      spawnEnemies("joc2d/data/guy.txt", guy);
      spawnEnemies("joc2d/data/turret.txt", turret);
      spawnEnemies("joc2d/data/jumper.txt", jumper);
      spawnEnemies("joc2d/data/shooter.txt", shooter);
    }

    boss = new FinalBoss;
    addModel(boss);
    addComponent(boss);
  }

  void registerEntity(Entity *ent) {
    shotComponent->addEntity(ent);
    forceShot->addEntity(ent);
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

      model->material->set("base", floorTexture);
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

BitMapFont *joc::font;
worldmats::Clock *joc::clock;
void setupBasic() {

  joc::clock = new worldmats::Clock;
  joc::clock->setName("Clock");
  shambhala::pushMaterial(joc::clock);
  shambhala::addComponent(joc::clock);

  ResWorldMat *worldMat = new ResWorldMat;
  shambhala::pushMaterial(worldMat);
  shambhala::addComponent(worldMat);

  joc::font = new BitMapFont("textures/font.png");
}

static void loadTestScene() {
  setupComponentSystem();
  setupLevel();
  setupBackground();
  setupShip();
}

static float getMenuScale() {
  float ra = viewport()->aspectRatio() * 0.6;
  return ra;
}

struct QuadArgs {

  glm::vec2 position;
  glm::vec2 size;
  glm::vec4 mul = glm::vec4(2.5, 2.5, 2.5, 1.0);
  glm::vec4 add = glm::vec4(0.0);
  glm::vec2 stOffset = glm::vec2(0.0);
  glm::vec2 stScale = glm::vec2(1.0);
  bool fit = true;

  void st(glm::vec2 offset, glm::vec2 scale) {
    stScale = scale;
    stOffset = offset;
  }
  void pos(glm::vec2 p, glm::vec2 size) {
    this->position = p;
    this->size = size;
  }

  bool inside(glm::vec2 p) {
    float ra = getMenuScale();
    glm::vec2 diff = size / glm::vec2(ra, 1.0);
    AABB abbb{position - diff, position + diff};
    return abbb.inside(p);
  }
};
static void drawQuad(shambhala::Texture *texture, QuadArgs args) {

  static shambhala::Program *tiled =
      loader::loadProgram("programs/tiled.fs", "programs/tiled.vs");
  static shambhala::Mesh *mesh = util::createTexturedQuad();

  float ra = getMenuScale();
  if (!args.fit)
    ra = 1.0f;
  tiled->use();
  tiled->bind("base", texture);
  tiled->bind(Standard::uProjectionMatrix, glm::mat4(1.0));
  tiled->bind(Standard::uViewMatrix, glm::mat4(1.0));

  glm::mat4 transform = util::translate(args.position.x, args.position.y, 0.0) *
                        util::scale(args.size.x / ra, args.size.y, 1.0) *
                        util::scale(2.0, 2.0, 1.0) *
                        util::translate(-0.5, -0.5, 0.0);

  tiled->bind(Standard::uTransformMatrix, transform);
  tiled->bind("mul", args.mul);
  tiled->bind("add", args.add);
  tiled->bind("stOffset", args.stOffset);
  tiled->bind("stScale", args.stScale);
  mesh->use();

  shambhala::device::drawCall();

  tiled->bind("stOffset", glm::vec2(0.0));
  tiled->bind("stScale", glm::vec2(1.0));
}

struct Button : public QuadArgs {
  bool pressed = false;
  bool hover;

  std::string text;

  void render(shambhala::Texture *back) {
    if (pressed) {
      st(glm::vec2(0.5, 0.0), glm::vec2(0.5, 1.0));
    } else {
      st(glm::vec2(0.0, 0.0), glm::vec2(0.5, 1.0));
    }

    if (hover) {
      add = glm::vec4(0.2, 0.4, 0.7, 0.2);
    } else {
      add = glm::vec4(0.0);
    }

    drawQuad(back, *this);

    joc::font->render(text,
                      this->position + glm::vec2(0.0, 0.02) -
                          (pressed ? glm::vec2(0.0, 0.05) : glm::vec2(0.0)),
                      glm::vec2(1.0 / getMenuScale(), 1.0));
  }

  void step(glm::vec2 mousep, bool press) {
    if (inside(mousep)) {
      hover = true;
      pressed = press;
    } else {
      hover = false;
      pressed = false;
    }
  }
};

struct Panel : public QuadArgs {
  std::string text;

  void render(shambhala::Texture *texture) {

    st(glm::vec2(0.0, 0.0), glm::vec2(0.5, 0.85));
    drawQuad(texture, *this);
    joc::font->render(text, this->position + glm::vec2(0.0, 0.02),
                      glm::vec2(1.0 / getMenuScale(), 1.0));
  }
};

static float creditsThreshold = 8.0;
static float gameThreshold = 5.0;
struct MenuComponent : public LogicComponent {

  shambhala::Texture *background = loader::loadTexture("textures/menu.png", 4);
  shambhala::Texture *back = loader::loadTexture("textures/back.png", 4);

  Button play, credits, end;
  Panel creditPanel;

  bool menu = true;
  bool game = false;
  bool credit = false;

  float creditsStep = 0.0;
  float gameStep = 0.0;

  bool enabled = true;
  bool stop = false;
  float delta = 0.1;

  MenuComponent() {
    background->useNeareast = true;
    back->useNeareast = true;
    back->clamp = true;
    play.text = "JUGAR";
    credits.text = "CREDITS";
    end.text = "END";
    creditPanel.text = "Programacio\n\nDavid garcia\nAlex\n\nGrafics\nGent "
                       "random\n\n3rd party\n\nWhatever";
  }

  void reset() {
    enabled = false;
    game = false;
    menu = true;
    credit = false;
    creditsStep = 0.0;
    gameStep = 0.0;
  }
  void render() override {

    if (enabled || stop) {
      QuadArgs args;

      float ty = glm::sin(0.5f * M_PI * gameStep / gameThreshold);
      float tx = glm::sin(0.5f * M_PI * creditsStep / creditsThreshold);

      float d = glm::max(3.0f - ty * 3.0f, stop ? 0.3f : 0.0f);
      args.fit = false;
      args.mul = glm::vec4(1.0, 1.0, 1.0, 0.0);
      args.add = glm::vec4(0.0, 0.0, 0.0, 0.8);

      args.pos(glm::vec2(0.0), glm::vec2(1.0));

      drawQuad(background, args);

      float creditDelta = 2.5f;
      float gameDelta = 3.0f;

      credits.pos(glm::vec2(-tx * creditDelta, -0.15 - ty * gameDelta),
                  glm::vec2(0.35, 0.15));
      end.pos(glm::vec2(-tx * creditDelta, -0.55 - ty * gameDelta),
              glm::vec2(0.35, 0.15));
      play.pos(glm::vec2(-tx * creditDelta, 0.25 - ty * gameDelta),
               glm::vec2(0.35, 0.15));

      creditPanel.pos(glm::vec2((1 - tx) * creditDelta, 0.0) + glm::vec2(0.0),
                      glm::vec2(0.8, 0.9));

      play.render(back);
      credits.render(back);
      end.render(back);
      creditPanel.render(back);

      if (credit) {
        creditsStep += delta;
        if (creditsStep >= creditsThreshold) {
          creditsStep = creditsThreshold;
        }
      }

      if (game && enabled) {
        gameStep += delta;
        if (gameStep >= gameThreshold) {
          gameStep = gameThreshold;
          reset();
          loadTestScene();
        }
      }

      glm::vec2 mousep = viewport()->getMouseViewportCoords() * 2.0f - 1.0f;
      mousep.y = -mousep.y;
      bool press = viewport()->isMousePressed();

      play.step(mousep, press);
      end.step(mousep, press);
      credits.step(mousep, press);

      if (credits.pressed) {
        credit = true;
      }
      if (play.pressed) {
        game = true;
      }
    }

    if (!enabled) {
      if (viewport()->isKeyJustPressed(KEY_ESCAPE)) {
        stop = !stop;
        reset();

        if (stop) {
          viewport()->deltaTime = 0.0;
        } else {
          viewport()->deltaTime = delta;
        }
      }
    }
  }

  void editorRender() override {
    if (ImGui::Begin("Menu Game")) {
      ImGui::Checkbox("Credit", &credit);
      ImGui::End();
    }
  }
};

void loadmenu() { addComponent(new MenuComponent); }
int main() {
  Joc joc;
  joc.enginecreate();
  setupBasic();
  loadmenu();
  // loadTestScene();
  joc.loop();
}
