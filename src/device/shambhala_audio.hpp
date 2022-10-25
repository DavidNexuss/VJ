#pragma once
#include "adapters/audio.hpp"
#include "core/core.hpp"
#include "shambhala.hpp"
#include <AL/al.h>

namespace shambhala {
namespace audio {

// Sound Components

struct SoundResource : public IResource {
  ALsizei size = 0;
  ALfloat freq = 0;
  ALenum format = 0;
  ALvoid *data = nullptr;
  ALboolean loop = AL_FALSE;

  io_buffer io = {};
  io_buffer *read() override;
};

struct SoundMesh {
  ResourceHandlerAbstract<SoundResource> soundResource;
  ALuint al_buffer = -1;
};

struct SoundModel {
  SoundMesh *mesh;
  Node *node = nullptr;

  // Update using node interpolation
  glm::vec3 velocity = glm::vec3(0.0);
  float pitch = 1.0;
  void play();

  ALuint al_source = -1;
};

SoundMesh *createSoundMesh();
SoundModel *createSoundModel();
void useSoundModel(SoundModel *model);
void useSoundMesh(SoundMesh *mesh);

SoundResource *createSoundResource(const char *name);
} // namespace audio

namespace util {
audio::SoundModel *spawnSoundModel(const char *name);
}
} // namespace shambhala
