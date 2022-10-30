#include "adapters/audio.hpp"

namespace shambhala {
struct AudioOpenAL : public shambhala::audio::IAudio {

  void destroyDevice() override;
  bool initDevice() override;
  void loadwave(const char *name, ALenum *format, ALvoid **data, ALsizei *size,
                ALfloat *freq, ALboolean *loop) override;
};
} // namespace shambhala
