#include "application.hpp"
#include "parallax.hpp"
#include "shambhala.hpp"
#include "tile.hpp"
#include <ext.hpp>
using namespace shambhala;

void loadTestScene() {

  Texture *baseColor = shambhala::createTexture();
  baseColor->useNeareast = true;
  baseColor->clamp = true;
  baseColor->addTextureResource(
      resource::stbiTextureFile("textures/green_tile.png", 4));

  TileMap *tiles = new TileMap(200, 20, new StaticTile, baseColor);
  shambhala::addComponent(tiles);
  tiles->loadLevel(resource::ioMemoryFile("levels/level01.txt"));

  ParallaxBackground *background = new ParallaxBackground;
  Texture *background1 = shambhala::createTexture();

  background->addParallaxBackground(
      1.0, loader::loadTexture("textures/cethiel_layer3.png", 4));
  background->addParallaxBackground(
      1.0, loader::loadTexture("textures/cethiel_layer2.png", 4));
  background->addParallaxBackground(
      1.0, loader::loadTexture("textures/cethiel_layer1.png", 4));

  // Adding fog
  Program *fog = loader::loadProgram("programs/fog.fs", "programs/parallax.vs");
  background->addParallaxBackground(
      1.0, loader::loadTexture("textures/fog.png", 4), 3, fog);
  shambhala::addComponent(background);

  worldmats::Clock *clock = new worldmats::Clock;
  clock->setName("Clock");
  shambhala::setWorldMaterial(2, clock);
  shambhala::addComponent(clock);
}
int main() {
  Joc joc;
  joc.enginecreate();
  loadTestScene();
  joc.loop();
}
