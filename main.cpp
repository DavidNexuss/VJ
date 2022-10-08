#include "adapters/log.hpp"
#include "ext/gi.hpp"
#include "ext/util.hpp"
#include "simple_vector.hpp"
#include <ext.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <impl/io_linux.hpp>
#include <impl/logger.hpp>
#include <impl/viewport_glfw.hpp>
#include <iostream>
#include <shambhala.hpp>
#include <standard.hpp>
using namespace shambhala;

simple_vector<TextureResource *>
textures(const simple_vector<const char *> &textures) {

  simple_vector<TextureResource *> result;
  for (int i = 0; i < textures.size(); i++) {
    result.push(shambhala::resource::stbiTextureFile(textures[i], 3));
  }
  return result;
}
Material *createSkyBox() {
  Model *skybox = util::modelCreateSkyBox(textures(
      {"assets/textures/canyon/right.png", "assets/textures/canyon/left.png",
       "assets/textures/canyon/top.png", "assets/textures/canyon/bottom.png",
       "assets/textures/canyon/front.png", "assets/textures/canyon/back.png"}));

  shambhala::addModel(skybox);
  return skybox->material;
}

Program *pbrProgram() {
  static Program *def = nullptr;
  if (def == nullptr) {

    def = createProgram();
    def->shaders[VERTEX_SHADER].file =
        resource::ioMemoryFile("programs/regular.vs");
    def->shaders[FRAGMENT_SHADER].file =
        resource::ioMemoryFile("programs/pbr.fs");
  }
  return def;
}

void setupModels() {

  Model *cube = createModel();
  cube->mesh = util::meshCreateCube();
  shambhala::addModel(cube);
}

Material *createPbrMaterial(const simple_vector<TextureResource *> &textureData,
                            Material *mat) {
  static const char *uniforms[] = {Standard::uBaseColor, Standard::uBump,
                                   Standard::uSpecial};
  for (int i = 0; i < textureData.size(); i++) {
    Texture *text = shambhala::createTexture();
    text->addTextureResource(textureData[i]);
    DynamicTexture dyn;
    dyn.sourceTexture = text;
    dyn.unit = Standard::tBaseColor + i;
    mat->set(std::string(uniforms[i]), dyn);
  }
  return mat;
}
#include <assimp/postprocess.h>
#include <standard.hpp>

Node *loadScene(const char *path) {
  SceneLoaderConfiguration configuration;
  configuration.combineMeshes = true;
  configuration.assimpFlags =
      aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_Triangulate |
      aiProcess_CalcTangentSpace | aiProcess_OptimizeGraph |
      aiProcess_OptimizeMeshes | aiProcess_PreTransformVertices;

  configuration.attributes = {{Standard::aPosition, 3},
                              {Standard::aNormal, 3},
                              {Standard::aUV, 2},
                              {Standard::aTangent, 3}};

  SceneDefinition def;
  def.scenePath = path;
  def.configuration = configuration;
  return loader::loadScene(path);
}
ModelList *setupObjects() {

  ModelList *weapons = shambhala::createModelList();
  shambhala::setWorkingModelList(weapons);
  Node *weapon = loadScene("machine/objects/weapon.obj");
  Material *mat =
      createPbrMaterial(textures({"textures/weapon/Weapon_BaseColor.png",
                                  "textures/weapon/Weapon_Normal.png",
                                  "textures/weapon/Weapon_Special.png"}),
                        shambhala::createMaterial());

  Program *program = shambhala::loader::loadProgram("programs/zdebug.fs",
                                                    "programs/regular.vs");
  program = pbrProgram();
  for (int i = 0; i < weapons->models.size(); i++) {
    weapons->models[i]->program = program;
    weapons->models[i]->material = mat;
  }

  return weapons;
  // gi::bakeAmbientOcclusion(weapon.models, 2048, 1);
}

/*
void setupPlayer() {
  Scene robot = loadScene("machine/objects/robot.obj");
  shambhala::setWorkingModelList(robot.models);
  Program *textured = shambhala::loader::loadProgram("programs/textured.fs",
                                                     "programs/regular.vs");
  Material *mat = shambhala::createMaterial();
  Texture *baseColor = shambhala::createTexture();
  baseColor->addTextureResource(
      shambhala::resource::stbiTextureFile("color2.png", 3));
  mat->set(Standard::uBaseColor, DynamicTexture{baseColor});

  Program *program = shambhala::loader::loadProgram("programs/zdebug.fs",
                                                    "programs/regular.vs");
  program = pbrProgram();

  shambhala::cea for (size_t i = 0; i < robot.models->size(); i++) {
    robot.models->get(i)->program = program;
    robot.models->get(i)->material = mat;
  }*/

//  gi::bakeAmbientOcclusion(robot.models, 2048,1);
//}

void enginecreate() {

  EngineParameters parameters;
  parameters.io = new shambhala::LinuxIO;
  parameters.viewport = new shambhala::ViewportGLFW;
  parameters.logger = new shambhala::DefaultLogger;
  parameters.io->translators.push_back("assets/%s");
  parameters.io->translators.push_back("internal_assets/%s");
  parameters.io->translators.push_back("machine/%s");
  shambhala::createEngine(parameters);

  WindowConfiguration configuration;
  configuration.titlename = "Test main";
  configuration.width = 800;
  configuration.height = 600;
  configuration.mssaLevel = 4;
  configuration.openglMajorVersion = 4;
  configuration.openglMinorVersion = 3;

  shambhala::setActiveWindow(shambhala::createWindow(configuration));
}

ModelList *loadModelList(const char *scenePath, Program *program,
                         Material *mat) {
  ModelList *scene = shambhala::createModelList();
  shambhala::setWorkingModelList(scene);

  Node *obj = shambhala::loader::loadScene(scenePath);

  for (int i = 0; i < scene->size(); i++) {
    scene->get(i)->program = program;
    scene->get(i)->material = mat;
  }

  shambhala::setWorkingModelList(nullptr);
  return scene;
}

Mesh *loadMesh(const char *scenePath) {
  ModelList *list = shambhala::createModelList();
  shambhala::setWorkingModelList(list);
  Node *scene = loader::loadScene(scenePath);
  Mesh *mesh = nullptr;
  for (int i = 0; i < list->size(); i++) {
    if (list->get(i) != nullptr)
      mesh = list->get(i)->mesh;
  }
  shambhala::disposeModelList(list);
  shambhala::setWorkingModelList(nullptr);
  return mesh;
}

ModelList *loadWeapon() {
  const char *path = "machine/objects/weapon.obj";
  Material *mat =
      createPbrMaterial(textures({"textures/weapon/Weapon_BaseColor.png",
                                  "textures/weapon/Weapon_Normal.png",
                                  "textures/weapon/Weapon_Special.png"}),
                        shambhala::createMaterial());

  Program *program = shambhala::loader::loadProgram("programs/zdebug.fs",
                                                    "programs/regular.vs");
  program = pbrProgram();
  return loadModelList(path, program, mat);
}

ModelList *loadSphere(Program *program, Material *mat) {
  return loadModelList("internal_assets/objects/giro.obj", program, mat);
}
ModelList *loadPlayer() {

  Program *program = shambhala::loader::loadProgram("programs/zdebug.fs",
                                                    "programs/regular.vs");
  program = pbrProgram();
  return loadModelList("machine/objects/robot.obj", program, nullptr);
}

ModelList *loadWorld() {
  ModelList *modelList = loadWeapon();
  shambhala::setWorkingModelList(modelList);
  shambhala::setWorldMaterial(Standard::wSky, createSkyBox());
  return modelList;
}
void loadDebugProbe(Node *rootNode) {
  Node *ringNode = shambhala::createNode("ringNode");
  Node *arrowNode = shambhala::createNode("arrowNode");

  Program *probe = shambhala::loader::loadProgram("programs/misc/probe.fs",
                                                  "programs/regular.vs");
  Material *red = shambhala::createMaterial();
  red->set("uColor", glm::vec4(1.0, 0.3, 0.3, 1.0));

  Material *blue = shambhala::createMaterial();
  blue->set("uColor", glm::vec4(0.3, 0.3, 1.0, 1.0));

  Material *green = shambhala::createMaterial();
  green->set("uColor", glm::vec4(0.3, 1.0, 0.3, 1.0));

  // Load rings
  {

    Mesh *ring = loadMesh("internal_assets/objects/giro.obj");
    ring->invertedFaces = false;
    Model *ringModel = shambhala::createModel();
    ringModel->mesh = ring;
    ringModel->program = probe;
    ringModel->node = shambhala::createNode();

    Model *ringy = ringModel->createInstance();
    ringy->material = green;
    shambhala::addModel(ringy);
    ringy->node->setName("ringy");

    Model *ringz = ringModel->createInstance();
    ringz->material = blue;
    ringz->node->setTransformMatrix(util::rotate(0, 0, 1, M_PI / 2));
    ringz->node->setName("ringz");
    shambhala::addModel(ringz);

    Model *ringx = ringz->createInstance();
    ringx->material = red;
    ringx->node->transform(util::rotate(0, 1, 0, M_PI / 2));
    ringx->node->setName("ringx");
    shambhala::addModel(ringx);

    ringx->node->setParentNode(ringNode);
    ringy->node->setParentNode(ringNode);
    ringz->node->setParentNode(ringNode);
  }

  // Load arrows
  {
    Mesh *arrow = loadMesh("internal_assets/objects/arrow.obj");
    arrow->invertedFaces = true;
    Model *arrowModel = shambhala::createModel();
    arrowModel->program = probe;
    arrowModel->node = shambhala::createNode();
    arrowModel->mesh = arrow;

    arrowModel->node->transform(util::scale(0.5));

    Model *arrowz = arrowModel->createInstance();
    arrowz->material = blue;
    arrowz->node->setName("arrowz");

    Model *arrowx = arrowModel->createInstance();
    arrowx->node->transform(util::rotate(0, 1, 0, M_PI / 2));
    arrowx->material = red;
    arrowx->node->setName("arrowx");

    Model *arrowy = arrowx->createInstance();
    arrowy->node->transform(util::rotate(1, 0, 0, -M_PI / 2));
    arrowy->material = green;
    arrowy->node->setName("arrowy");

    shambhala::addModel(arrowz);
    shambhala::addModel(arrowx);
    shambhala::addModel(arrowy);

    arrowz->node->setParentNode(arrowNode);
    arrowx->node->setParentNode(arrowNode);
    arrowy->node->setParentNode(arrowNode);
  }

  ringNode->setEnabled(false);
  ringNode->setParentNode(rootNode);
  arrowNode->setParentNode(rootNode);
}
int main() {

  enginecreate();
  RenderCamera *renderCamera = shambhala::createRenderCamera();
  RenderShot shot;
  shambhala::setWorkingModelList(shambhala::createModelList());
  shot.scenes = {shambhala::getWorkingModelList()};

  loadDebugProbe(shambhala::createNode("debugProbe"));

  shambhala::setWorldMaterial(Standard::wSky, createSkyBox());
  shambhala::setWorldMaterial(Standard::wCamera, new worldmats::DebugCamera);

  editor::editorInit();
  int frame = 0;
  do {
    shambhala::loop_beginRenderContext();
    renderCamera->render(shot);

    shambhala::loop_beginUIContext();
    editor::editorBeginContext();
    editor::editorRender(shot.currentFrame);
    editor::editorEndContext();
    shambhala::loop_endUIContext();

    shambhala::loop_shouldClose();
    shambhala::loop_endRenderContext();

    shot.updateFrame();

  } while (!shambhala::loop_shouldClose());
}
