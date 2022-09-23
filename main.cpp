#include "adapters/log.hpp"
#include "ext/util.hpp"
#include "simple_vector.hpp"
#include <ext.hpp>
#include <impl/io_linux.hpp>
#include <impl/logger.hpp>
#include <impl/viewport_glfw.hpp>
#include <iostream>
#include <shambhala.hpp>
#include <standard.hpp>
using namespace shambhala;

void setupRender() {
  Material *camera = new worldmats::DebugCamera;
  shambhala::setWorldMaterial(0, camera);
}

simple_vector<TextureResource *>
textures(const simple_vector<const char *> &textures) {

  simple_vector<TextureResource *> result;
  for (int i = 0; i < textures.size(); i++) {
    result.push(shambhala::resource::stbiTextureFile(textures[i], 3));
  }
  return result;
}
void createSkyBox() {
  Model *skybox = util::modelCreateSkyBox(textures(
      {"assets/textures/canyon/right.png", "assets/textures/canyon/left.png",
       "assets/textures/canyon/top.png", "assets/textures/canyon/bottom.png",
       "assets/textures/canyon/front.png", "assets/textures/canyon/back.png"}));

  shambhala::addModel(skybox);
}

Program *errorProgram;

Program *deferredProgram() {
  static Program *deferredProgram = nullptr;
  if (deferredProgram == nullptr) {

    deferredProgram = createProgram();
    deferredProgram->shaders[VERTEX_SHADER].file =
        resource::ioMemoryFile("programs/forward_pbr.vs");
    deferredProgram->shaders[FRAGMENT_SHADER].file =
        resource::ioMemoryFile("programs/forward_pbr.fs");
  }
  return deferredProgram;
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
void setupPrograms() {

  errorProgram = createProgram();
  errorProgram->shaders[FRAGMENT_SHADER].file =
      resource::ioMemoryFile("assets/materials/error.frag");
  errorProgram->shaders[VERTEX_SHADER].file =
      resource::ioMemoryFile("assets/materials/error.vert");
}
Model *cube;
void setupModels() {

  cube = createModel();
  cube->mesh = util::meshCreateCube();
  cube->program = errorProgram;
  shambhala::addModel(cube);
}

Material *
createDefferedMaterial(const simple_vector<TextureResource *> &textureData) {
  Material *mat = shambhala::createMaterial();
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

void setupObjects() {
  SceneLoaderConfiguration configuration;
  configuration.assimpFlags = aiProcess_FlipUVs | aiProcess_GenNormals |
                              aiProcess_Triangulate |
                              aiProcess_CalcTangentSpace;
  configuration.attributes = {{Standard::aPosition, 3},
                              {Standard::aNormal, 3},
                              {Standard::aUV, 2},
                              {Standard::aTangent, 3}};

  SceneDefinition def;
  def.scenePath = "machine/objects/weapon.obj";
  def.configuration = configuration;
  Scene scene(def);
  shambhala::setWorkingModelList(scene.models);
  Material *mat =
      createDefferedMaterial(textures({"textures/weapon/Weapon_BaseColor.png",
                                       "textures/weapon/Weapon_Normal.png",
                                       "textures/weapon/Weapon_Special.png"}));

  for (int i = 0; i < scene.models->models.size(); i++) {
    scene.models->models[i]->program = pbrProgram();
    scene.models->models[i]->material = mat;
  }
}
int main() {
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
  shambhala::rendertarget_prepareRender();

  setupRender();
  setupPrograms();
  setupObjects();

  // Init
  createSkyBox();
  // setupModels();

  // shambhala::setRootRenderCamera(rendercamera::createBlendPass(rendercamera::createForwardPass()));
  int frame = 0;

  // setupRenderPipeline();
  do {

    shambhala::loop_step();
    shambhala::loop_beginRenderContext();
    shambhala::loop_declarativeRender();
    shambhala::loop_beginUIContext();
    shambhala::loop_endUIContext();
    shambhala::loop_endRenderContext();

  } while (!shambhala::loop_shouldClose());
}
