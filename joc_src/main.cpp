#include "application.hpp"
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
}
int main() {
  Joc joc;
  joc.enginecreate();
  loadTestScene();
  joc.loop();
}
