#include <shambhala.hpp>

struct ParallaxBackground : public shambhala::LogicComponent {

  ParallaxBackground();
  void addParallaxBackground(float speed, shambhala::Texture *texture);

  void editorRender() override;

private:
  int texturecount = 0;
  shambhala::Program *parallaxProgram = nullptr;
  shambhala::Mesh *parallaxMesh = nullptr;
  simple_vector<shambhala::Model *> models;
};
