#include <shambhala.hpp>
namespace shambhala {
namespace rendercamera {
RenderCamera *createDefferedPass();
RenderCamera *createForwardPass();
RenderCamera *pbrPass(RenderCamera *deferredPass);
RenderCamera *gaussPass(RenderCamera *camera, int attachment);
RenderCamera *createBlendPass(RenderCamera *pbrPass);
} // namespace rendercamera
} // namespace shambhala
