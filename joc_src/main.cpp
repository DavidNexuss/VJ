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

  TileMap *tiles = new TileMap(32, new StaticTile, baseColor);
  tiles->set(0, 0, 1);
  tiles->set(1, 0, 2);
  tiles->set(2, 0, 2);
  tiles->set(3, 0, 2);
  tiles->set(4, 0, 3);
  shambhala::addComponent(tiles);
}
int main() {
  Joc joc;
  joc.enginecreate();
  loadTestScene();
  joc.loop();
}
