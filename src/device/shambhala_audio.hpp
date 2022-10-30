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
  bool loop = false;
  void play();

  ALuint al_source = -1;

  ~SoundModel();
};

struct SoundListener {
  glm::vec3 position = glm::vec3(0, 0, 1);
  glm::vec3 velocity = glm::vec3(0, 0, 0);
  struct {
    glm::vec3 x = glm::vec3(1, 0, 0);
    glm::vec3 y = glm::vec3(0, 1, 0);
  } orientation;

  void use();
};

namespace device {
bool isPlaying(ALuint source);
void diposeSource(ALuint source);
void step_dispose();
} // namespace device

SoundMesh *createSoundMesh();
SoundModel *createSoundModel();
void useSoundModel(SoundModel *model);
void useSoundMesh(SoundMesh *mesh);

void loop_stepAudio();

SoundResource *createSoundResource(const char *name);
} // namespace audio

namespace util {
audio::SoundModel *spawnSoundModel(const char *name);
}
} // namespace shambhala
