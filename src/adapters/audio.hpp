#pragma once
#include "adapters/log.hpp"
#include <AL/al.h>
#include <glm/glm.hpp>

struct ALCdevice;
namespace shambhala {
namespace audio {
inline ALCdevice *debugDevice = nullptr;
}
} // namespace shambhala
#define ALC(X)                                                                 \
  {                                                                            \
    alGetError();                                                              \
    { X; }                                                                     \
    auto error = alGetError();                                                 \
    if (error != AL_NO_ERROR) {                                                \
      LOG("Error %p %s", audio::debugDevice,                                   \
          alcGetString(audio::debugDevice, error));                            \
    }                                                                          \
  }
namespace shambhala {
namespace audio {
struct Listener {
  glm::vec3 position;
  glm::vec3 velocity;
  struct {
    glm::vec3 x;
    glm::vec3 y;
  } orientation;
};
struct IAudio {
  virtual bool initDevice() = 0;
  virtual void destroyDevice() = 0;
  virtual void configureListener(Listener listener) = 0;
  virtual void loadwave(const char *name, ALenum *format, ALvoid **data,
                        ALsizei *size, ALfloat *freq, ALboolean *loop) = 0;
};
} // namespace audio
} // namespace shambhala
