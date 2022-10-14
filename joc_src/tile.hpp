#pragma once
#include <shambhala.hpp>

struct Tile {
  float xstart;
  float ystart;
  float xend;
  float yend;
};

struct TileAtlas {
  virtual Tile getTile(int tileindex) = 0;
};

struct StaticTile : public TileAtlas {
  std::unordered_map<int, Tile> tiles;
  Tile getTile(int tileindex) override;
};

struct TileAttribute {
  glm::vec3 position;
  glm::vec2 uv;
};

struct TileMap : public shambhala::LogicComponent {
  TileMap(int sizex, int sizey, TileAtlas *atlas, shambhala::Texture *text);

private:
  TileAtlas *atlas;
  shambhala::Model *model;
  shambhala::Texture *text;
  simple_vector<int> tiles;
  int sizex;
  int sizey;
  bool needsUpdate = false;

  static int spawnFace(simple_vector<TileAttribute> &vertexBuffer,
                       simple_vector<int> &indexBuffer, int count, Tile tile,
                       float x, float y, float p, int orientation);

  static int spawnTile(simple_vector<TileAttribute> &vertexBuffer,
                       simple_vector<int> &indexBuffer, int count, Tile tile,
                       int x, int y);

  static int spawnVoxel(simple_vector<TileAttribute> &vertexBuffer,
                        simple_vector<int> &indexBuffer, int count, Tile tile,
                        int x, int y);

  void updateMesh();

  // editor stuff
  int editorMaterial = 1;

public:
  void set(int i, int j, int mat);
  void step(shambhala::StepInfo info) override;
  void editorStep(shambhala::StepInfo info) override;
  void editorRender() override;
  void serialize();
};
