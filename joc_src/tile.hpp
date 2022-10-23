#pragma once
#include "atlas.hpp"
#include "entity/entity.hpp"
#include <shambhala.hpp>

struct TileAttribute {
  glm::vec3 position;
  glm::vec2 uv;
};

struct TileBake {
  GLuint bakedTexture = -1;
  glm::vec2 size;
};

struct TileMap : public shambhala::LogicComponent, public Entity {
  TileMap(int sizex, int sizey, TileAtlas *atlas, shambhala::Texture *text,
          int zindex);

  shambhala::Node *rootNode = nullptr;

private:
  int zindex = 10;
  // Baking

  shambhala::FrameBuffer *fbo = nullptr;
  shambhala::FrameBuffer *bake_fbo = nullptr;

  // Resources
  ResourceHandler levelResource;
  ResourceHandler levelResourceEditor;

  TileAtlas *atlas;
  shambhala::Texture *textureAtlas;

  shambhala::Model *model = nullptr;
  shambhala::Model *bakedModel = nullptr;
  shambhala::Model *illuminationModel = nullptr;

  TileBake bakeInformation;
  TileBake bakeInformationShadow;
  simple_vector<int> tiles;
  int bakeCount = 0;
  int sizex;
  int sizey;

  bool needsUpdate = false;

  void enableBake(bool pEnable);
  void bake();
  void bakeShadows();
  void initmap(IResource *resource);
  void updateMesh();

  // editor stuff
  int editorMaterial = 1;

public:
  int get(int i, int j);
  void set(int i, int j, int mat);
  void step(shambhala::StepInfo info) override;
  void editorStep(shambhala::StepInfo info) override;
  void editorRender() override;
  std::string serialize();
  void save();
  void loadLevel(IResource *leveldata);

  bool inside(glm::vec2 lowerCorner, glm::vec2 highCorner) override;
  bool inside(glm::vec2 position) override;
  void signalHit() override;
};
