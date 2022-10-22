#include "atlas.hpp"

Tile StaticTileAtlas::getTile(int tileindex) {
  Tile tile;
  float p = 16.0f / 512.0f;
  tile.xstart = (tileindex % 32) * p;
  tile.ystart = float(tileindex / 32) * p;
  tile.xend = tile.xstart + p;
  tile.yend = tile.ystart + p;
  std::swap(tile.yend, tile.ystart);
  return tile;
}

Tile DynamicTileAtlas::getTile(int tileindex) {
  Tile tile;
  tile.xstart = coords[tileindex * 4];
  tile.ystart = coords[tileindex * 4 + 1];
  tile.xend = coords[tileindex * 4 + 2] + tile.xstart;
  tile.yend = coords[tileindex * 4 + 3] + tile.ystart;

  tile.xstart /= this->atlasWidth;
  tile.ystart /= this->atlasHeight;
  tile.xend /= this->atlasWidth;
  tile.yend /= this->atlasHeight;

  return tile;
}
