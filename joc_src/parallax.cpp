#include "parallax.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
using namespace shambhala;

ParallaxBackground::ParallaxBackground() {
  parallaxProgram =
      loader::loadProgram("programs/parallax.fs", "programs/parallax.vs");
  parallaxMesh = util::createScreen();
  setName("ParallaxMap");
}
void ParallaxBackground::addParallaxBackground(float speed,
                                               shambhala::Texture *texture) {

  Model *parallaxBackground = shambhala::createModel();
  parallaxBackground->mesh = parallaxMesh;
  parallaxBackground->program = parallaxProgram;
  parallaxBackground->material = shambhala::createMaterial();
  parallaxBackground->depthMask = true;
  parallaxBackground->zIndex = -(texturecount + 1);

  DynamicTexture dyn;
  dyn.sourceTexture = texture;
  dyn.unit = texturecount;

  shambhala::setupMaterial(parallaxBackground->material, parallaxProgram);
  parallaxBackground->material->set("input", dyn);
  models.push(parallaxBackground);
  shambhala::addModel(parallaxBackground);
  texturecount++;
}
