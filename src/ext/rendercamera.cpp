#include "rendercamera.hpp"
#include "resource.hpp"
#include "util.hpp"
#include <standard.hpp>

using namespace shambhala;
using namespace rendercamera;
namespace shambhala {
RenderCamera *rendercamera::createDefferedPass() {
  RenderCamera *deferredPipeline = shambhala::createRenderCamera();
  deferredPipeline->addOutput({GL_RGB, GL_RGB, GL_UNSIGNED_BYTE});   // ALBEDO
  deferredPipeline->addOutput({GL_RGB, GL_RGB, GL_FLOAT});           // NORMAL
  deferredPipeline->addOutput({GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE}); // ESP
  deferredPipeline->setFrameBufferConfiguration(shambhala::USE_DEPTH);
  return deferredPipeline;
}

RenderCamera *rendercamera::createForwardPass() {

  RenderCamera *pbrPass = shambhala::createRenderCamera();
  pbrPass->addOutput({GL_RGB, GL_RGB, GL_FLOAT});
  pbrPass->addOutput({GL_RGB, GL_RGB, GL_FLOAT});
  pbrPass->setFrameBufferConfiguration(shambhala::USE_RENDER_BUFFER |
                                       shambhala::USE_DEPTH);
  return pbrPass;
}

RenderCamera *rendercamera::pbrPass(RenderCamera *deferredPass) {
  RenderCamera *pbrPass = shambhala::createRenderCamera();

  pbrPass->addInput(deferredPass, 0, Standard::uBaseColor);
  pbrPass->addInput(deferredPass, 1, Standard::uBump);
  pbrPass->addInput(deferredPass, 2, Standard::uSpecial);
  pbrPass->addInput(deferredPass, Standard::attachmentDepthBuffer,
                    Standard::uDepth);

  pbrPass->addOutput({GL_RGB, GL_RGB, GL_FLOAT});
  pbrPass->addOutput({GL_RGB, GL_RGB, GL_FLOAT});

  pbrPass->postprocessProgram = shambhala::util::createScreenProgram(
      resource::ioMemoryFile("programs/pbr.fs"));
  return pbrPass;
}

RenderCamera *rendercamera::gaussPass(RenderCamera *camera, int attachment) {
  RenderCamera *gaussPass = shambhala::createRenderCamera();
  gaussPass->postprocessProgram =
      util::createScreenProgram(resource::ioMemoryFile("programs/gauss.fs"));
  gaussPass->addInput(camera, attachment, "image");
  gaussPass->addOutput({GL_RGB, GL_RGB, GL_FLOAT});
  return gaussPass;
}

RenderCamera *rendercamera::createBlendPass(RenderCamera *pbrPass) {

  RenderCamera *blendPass = shambhala::createRenderCamera();
  blendPass->addInput(pbrPass, 0, "scene");
  blendPass->addInput(gaussPass(pbrPass, 1), 0, "bloom");
  blendPass->addOutput({GL_RGB, GL_RGB, GL_UNSIGNED_BYTE});
  blendPass->postprocessProgram = shambhala::util::createScreenProgram(
      resource::ioMemoryFile("programs/blend.fs"));
  return blendPass;
}
} // namespace shambhala
