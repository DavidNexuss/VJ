#pragma once
#include "atlas.hpp"
#include "entity/entity.hpp"
#include <shambhala.hpp>

struct TileMap : public shambhala::LogicComponent, public Entity {
  TileMap(int sizex, int sizey, TileAtlas *atlas, shambhala::Texture *text,
          int zindex);

  shambhala::Node *rootNode = nullptr;

private:
  int zindex = 10;
  // Baking

  shambhala::FrameBuffer *fbo_shadows = nullptr;
  shambhala::FrameBuffer *fbo_bake = nullptr;

  // Resources
  ResourceHandler levelResource;
  ResourceHandler levelResourceEditor;

  TileAtlas *atlas;
  shambhala::Texture *textureAtlas;

  shambhala::Model *model = nullptr;
  shambhala::Model *bakedModel = nullptr;
  shambhala::Model *illuminationModel = nullptr;

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

  Collision inside(glm::vec2 p) override;
  Collision inside(AABB collisionBox) override;
  void signalHit(Collision col) override;
};
