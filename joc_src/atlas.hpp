#pragma once
#include <unordered_map>

struct Tile {
  float xstart;
  float ystart;
  float xend;
  float yend;
};

struct TileAtlas {
  virtual Tile getTile(int tileindex) = 0;
};

struct StaticTileAtlas : public TileAtlas {
  std::unordered_map<int, Tile> tiles;
  Tile getTile(int tileindex) override;
};

struct DynamicTileAtlas : public TileAtlas {
  int *coords;
  float atlasWidth;
  float atlasHeight;

  Tile getTile(int tileindex) override;
};
