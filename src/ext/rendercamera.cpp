#include "rendercamera.hpp"
#include <standard.hpp>

using namespace rendercamera;
RenderCamera *rendercamera::createDefferedPass() {
  RenderCamera *deferredPipeline = RenderCamera::create();
  deferredPipeline->addOutput({GL_RGB, GL_RGB, GL_UNSIGNED_BYTE});   // ALBEDO
  deferredPipeline->addOutput({GL_RGB, GL_RGB, GL_FLOAT});           // NORMAL
  deferredPipeline->addOutput({GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE}); // ESP
  deferredPipeline->setConfiguration(USE_DEPTH);
  return deferredPipeline;
}

RenderCamera *rendercamera::createForwardPass() {

  RenderCamera *pbrPass = RenderCamera::create();
  pbrPass->addOutput({GL_RGB, GL_RGB, GL_FLOAT});
  pbrPass->addOutput({GL_RGB, GL_RGB, GL_FLOAT});
  pbrPass->setConfiguration(USE_RENDER_BUFFER | USE_DEPTH);
  return pbrPass;
}

RenderCamera *rendercamera::pbrPass(RenderCamera *deferredPass) {
  RenderCamera *pbrPass = new PostProcessCamera("pbr.fs");

  pbrPass->set(Standard::uBaseColor, deferredPass->renderOutput(0));
  pbrPass->set(Standard::uBump, deferredPass->renderOutput(1));
  pbrPass->set(Standard::uSpecial, deferredPass->renderOutput(2));
  pbrPass->set(Standard::uDepth,
               deferredPass->renderOutput(Standard::attachmentDepthBuffer));

  pbrPass->addOutput({GL_RGB, GL_RGB, GL_FLOAT});
  pbrPass->addOutput({GL_RGB, GL_RGB, GL_FLOAT});

  return pbrPass;
}

RenderCamera *rendercamera::gaussPass(RenderCamera *camera, int attachment) {

  RenderCamera *gaussPass = new PostProcessCamera("pbr.fs");
  gaussPass->set("image", camera->renderOutput(attachment));
  gaussPass->addOutput({GL_RGB, GL_RGB, GL_FLOAT});
  return gaussPass;
}

RenderCamera *rendercamera::createBlendPass(RenderCamera *pbrPass) {

  RenderCamera *blendPass = new PostProcessCamera("blend.fs");
  blendPass->set("scene", pbrPass->renderOutput(0));
  blendPass->set("bloom", pbrPass->renderOutput(1));
  blendPass->addOutput({GL_RGB, GL_RGB, GL_UNSIGNED_BYTE});
  return blendPass;
}
