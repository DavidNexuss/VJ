#include "adapters/log.hpp"
#include "ext/util.hpp"
#include "simple_vector.hpp"
#include <ext.hpp>
#include <impl/io_linux.hpp>
#include <impl/logger.hpp>
#include <impl/viewport_glfw.hpp>
#include <iostream>
#include <shambhala.hpp>
using namespace shambhala;

void setupRender() {
  worldmats::Camera *camera = new worldmats::DebugCamera;
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
void setup() {

  createSkyBox();
  Model *cube = createModel();
  cube->program = createProgram();
  cube->program->shaders[FRAGMENT_SHADER].file =
      resource::ioMemoryFile("assets/materials/error.frag");
  cube->program->shaders[VERTEX_SHADER].file =
      resource::ioMemoryFile("assets/materials/error.vert");
  cube->mesh = util::meshCreateCube();
  shambhala::addModel(cube);
}

int main() {
  EngineParameters parameters;
  parameters.io = new shambhala::LinuxIO;
  parameters.viewport = new shambhala::ViewportGLFW;
  parameters.logger = new shambhala::DefaultLogger;
  parameters.io->translators.push_back("assets/%s");
  shambhala::createEngine(parameters);

  WindowConfiguration configuration;
  configuration.titlename = "Test main";
  configuration.width = 800;
  configuration.height = 600;
  configuration.mssaLevel = 2;
  configuration.openglMajorVersion = 4;
  configuration.openglMinorVersion = 3;

  shambhala::setActiveWindow(shambhala::createWindow(configuration));
  setupRender();
  setup();
  do {

    shambhala::loop_step();
    shambhala::loop_beginRenderContext();
    shambhala::loop_declarativeRender();
    shambhala::loop_beginUIContext();
    shambhala::loop_endUIContext();
    shambhala::loop_endRenderContext();

  } while (!shambhala::loop_shouldClose());
}
