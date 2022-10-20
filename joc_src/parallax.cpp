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
void ParallaxBackground::addParallaxBackground(
    float speed, shambhala::Texture *texture, int zIndex,
    shambhala::Program *customProgram) {

  Model *parallaxBackground = shambhala::createModel();
  parallaxBackground->mesh = parallaxMesh;
  parallaxBackground->program = parallaxProgram;

  if (customProgram != nullptr) {
    parallaxBackground->program = customProgram;
  }
  parallaxBackground->material = shambhala::createMaterial();
  shambhala::setupMaterial(parallaxBackground->material,
                           parallaxBackground->program);

  parallaxBackground->depthMask = true;
  if (zIndex == 0)
    parallaxBackground->zIndex = -(texturecount + 1);
  else
    parallaxBackground->zIndex = zIndex;

  DynamicTexture dyn;
  dyn.sourceTexture = texture;
  dyn.unit = texturecount;

  parallaxBackground->material->set("input", dyn);
  models.push(parallaxBackground);
  shambhala::addModel(parallaxBackground);
  texturecount++;
}
