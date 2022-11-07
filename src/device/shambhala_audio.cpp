#include "shambhala_audio.hpp"
#include "shambhala.hpp"
#include <AL/al.h>
#include <AL/alc.h>

using namespace shambhala;
using namespace shambhala::audio;

#undef ALC
#define ALC(x)                                                                 \
  {}
struct SoundBindState {
  simple_vector<ALuint> toRemoveSources;
};

static SoundBindState sound_bindState;

void audio::device::diposeSource(ALuint source) {
  sound_bindState.toRemoveSources.push(source);
}

bool audio::device::isPlaying(ALuint source) {
  ALenum state;
  ALC(alGetSourcei(source, AL_SOURCE_STATE, &state));
  return (state == AL_PLAYING);
}

void audio::device::step_dispose() {
  static simple_vector<ALuint> toRemove;
  toRemove.clear();

  for (int i = 0; i < sound_bindState.toRemoveSources.size(); i++) {
    if (!isPlaying(sound_bindState.toRemoveSources[i])) {
      toRemove.push(sound_bindState.toRemoveSources[i]);
      sound_bindState.toRemoveSources.removeNShift(i);
      i--;
    }
  }

  if (toRemove.size())
    ALC(alDeleteSources((ALuint)toRemove.size(), &toRemove[0]));
}

io_buffer *SoundResource::read() {
  aud()->loadwave(resourcename.c_str(), &format, &data, &size, &freq, &loop);
  io.data = (uint8_t *)data;
  io.length = size;
  return &io;
}

void audio::useSoundModel(SoundModel *model) {
  if (model->al_source == -1) {
    ALC(alGenSources((ALuint)1, &model->al_source));
  }
  if (model->node == nullptr)
    model->node = shambhala::createNode();

  glm::vec3 position =
      model->node->getCombinedMatrix() * glm::vec4(glm::vec3(0.0), 1.0);

  ALC(alSourcef(model->al_source, AL_PITCH, model->pitch));
  ALC(alSourcef(model->al_source, AL_GAIN, 1.0));
  ALC(alSource3f(model->al_source, AL_POSITION, position.x, position.y,
                 position.z));

  ALC(alSource3f(model->al_source, AL_VELOCITY, model->velocity.x,
                 model->velocity.y, model->velocity.z));
  ALC(alSourcei(model->al_source, AL_LOOPING,
                model->loop ? AL_TRUE : AL_FALSE));

  useSoundMesh(model->mesh);
}

SoundMesh *audio::createSoundMesh() { return new SoundMesh; }
SoundModel *audio::createSoundModel() { return new SoundModel; }

void audio::useSoundMesh(SoundMesh *mesh) {

  // No resource bound!!
  if (!mesh->soundResource.cleanFile())
    return;

  if (mesh->al_buffer == -1) {
    ALC(alGenBuffers((ALuint)1, &mesh->al_buffer));
  }

  SoundResource *file = mesh->soundResource.file();
  if (file) {
    file->read();
    ALC(alBufferData(mesh->al_buffer, file->format, file->data, file->size,
                     file->freq));

    mesh->soundResource.signalAck();
  }
}

void SoundModel::play() {
  useSoundModel(this);
  ALC(alSourcei(al_source, AL_BUFFER, mesh->al_buffer));
  ALC(alSourcePlay(al_source));
}

SoundResource *audio::createSoundResource(const char *name) {
  SoundResource *resource = new SoundResource;
  resource->resourcename = std::string(name);
  resource->signalUpdate();
  return resource;
}

audio::SoundModel *util::spawnSoundModel(const char *name) {
  SoundModel *model = audio::createSoundModel();
  model->mesh = audio::createSoundMesh();
  model->mesh->soundResource.acquire(audio::createSoundResource(name));
  model->node = shambhala::createNode();
  return model;
}

SoundModel::~SoundModel() {
  if (al_source != -1) {
    audio::device::diposeSource(al_source);
  }
}

void SoundListener::use() {
  ALC(alListener3f(AL_POSITION, position.x, position.y, position.z));
  ALC(alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z));
  ALC(alListenerfv(AL_ORIENTATION, (ALfloat *)&orientation));
}
void audio::loop_stepAudio() { device::step_dispose(); }
