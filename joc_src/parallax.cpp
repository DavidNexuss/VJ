#include "parallax.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
using namespace shambhala;

ParallaxBackground::ParallaxBackground() {
  parallaxProgram =
      loader::loadProgram("programs/parallax.fs", "programs/parallax.vs");
  parallaxMesh = util::createScreen();
  setName("ParallaxMap");
  parallaxNode = shambhala::createNode();
  parallaxNode->setName("parallaxNode");
}
shambhala::Material *ParallaxBackground::addParallaxBackground(
    const char *name, float speed, shambhala::Texture *texture, int zIndex,
    shambhala::Program *customProgram) {

  Model *parallaxBackground = shambhala::createModel();
  parallaxBackground->mesh = parallaxMesh;
  parallaxBackground->program = parallaxProgram;
  parallaxBackground->node = shambhala::createNode();
  parallaxBackground->node->setParentNode(parallaxNode);
  if (customProgram != nullptr) {
    parallaxBackground->program = customProgram;
  }
  parallaxBackground->material = shambhala::createMaterial();
  parallaxBackground->material->setSetupProgram(parallaxBackground->program);

  parallaxBackground->depthMask = true;
  if (zIndex == 0)
    parallaxBackground->zIndex = -(texturecount + 1) - 4;
  else
    parallaxBackground->zIndex = zIndex;

  parallaxBackground->material->set("input1", texture);
  models.push(parallaxBackground);
  shambhala::addModel(parallaxBackground);
  texturecount++;
  return parallaxBackground->material;
}
