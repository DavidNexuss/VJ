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

Scene loadScene(const char *path) {
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
  return Scene(def);
}
ModelList *setupObjects() {

  ModelList *weapons = shambhala::createModelList();
  shambhala::setWorkingModelList(weapons);
  Scene weapon = loadScene("machine/objects/weapon.obj");
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

  Scene *obj = shambhala::loader::loadScene(scenePath);

  for (int i = 0; i < scene->size(); i++) {
    scene->get(i)->program = program;
    scene->get(i)->material = mat;
  }

  shambhala::setWorkingModelList(nullptr);
  return scene;
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
int main() {

  enginecreate();
  RenderCamera *renderCamera = shambhala::createRenderCamera();
  renderCamera->addOutput({GL_RGB, GL_RGB, GL_UNSIGNED_BYTE});
  shambhala::setWorldMaterial(Standard::wCamera, new worldmats::DebugCamera);

  int frame = 0;

  // Program *probe =
  // shambhala::loader::loadProgram("programs/misc/probe.fs","programs/regular.vs");
  ModelList *scene = loadWeapon();
  shambhala::setWorkingModelList(scene);

  shambhala::setWorldMaterial(Standard::wSky, createSkyBox());
  RenderCamera *passThroughCamera = util::createPassThroughCamera(renderCamera);

  renderCamera->setSize(200, 100);
  renderCamera->setModelList(getWorkingModelList());
  do {
    shambhala::loop_beginRenderContext();

    passThroughCamera->render(frame++, true);
    /*
    shambhala::loop_beginUIContext();
    editor::editorRender(frame++);
    shambhala::loop_endUIContext(); */

    // renderCamera->render(frame++, true);

    shambhala::loop_endRenderContext();

  } while (!shambhala::loop_shouldClose());
}
