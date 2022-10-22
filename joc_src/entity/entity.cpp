#include "entity.hpp"
#include "ext/util.hpp"
#include "shambhala.hpp"
#include <ext.hpp>
using namespace shambhala;

void DynamicPartAtlas::configureNode(int part, Node *node) {

  glm::vec2 scale = glm::vec2(coords[part * 4 + 2], coords[part * 4 + 3]);
  glm::mat4 transform = node->getTransformMatrix();
  transform[0].x = scale.x / 32.0f;
  transform[1].y = scale.y / 32.0f;
  node->setTransformMatrix(transform);
}
void DynamicPartAtlas::configureMaterial(int part, Material *material) {
  DynamicTexture dyn;
  dyn.sourceTexture = textureAtlas;
  dyn.unit = 0;

  glm::vec2 textureSize =
      glm::vec2(textureAtlas->textureData[0].cleanFile()->width,
                textureAtlas->textureData[0].cleanFile()->height);

  glm::vec2 offset = glm::vec2(coords[part * 4], coords[part * 4 + 1]);
  glm::vec2 scale = glm::vec2(coords[part * 4 + 2], coords[part * 4 + 3]);
  offset /= textureSize;
  scale /= textureSize;

  material->set("input", dyn);
  material->set("uv_scale", scale);
  material->set("uv_offset", offset);
  material->set("textureSize", textureSize);
}

static Model *createStandardEntity() {
  Model *model = shambhala::createModel();
  model->node = shambhala::createNode();
  model->mesh = util::createTexturedQuad();
  model->zIndex = -1;
  return model;
}
Model *DynamicPartAtlas::createDynamicPart(int part) {
  Model *model = shambhala::createModel();
  model->node = shambhala::createNode();
  model->mesh = util::createTexturedQuad();
  model->material = shambhala::createMaterial();
  model->program = renderingProgram;
  configureMaterial(part, model->material);
  configureNode(part, model->node);
  model->node->setName((std::string("Part ") + std::to_string(part)).c_str());
  model->zIndex = -1;
  return model;
}

Model *DynamicPartAtlas::createLight() {
  Model *model = createStandardEntity();
  model->program =
      loader::loadProgram("programs/light.fs", "programs/regular.vs");
  model->node->setName("Light");
  return model;
}

DynamicPartAtlas *DynamicPartAtlas::create(shambhala::Program *program,
                                           shambhala::Texture *text,
                                           int *corrds) {
  DynamicPartAtlas *atlas = new DynamicPartAtlas;
  atlas->renderingProgram = program;
  atlas->textureAtlas = text;
  atlas->coords = corrds;
  return atlas;
}
