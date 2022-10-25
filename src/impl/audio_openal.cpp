#include "audio_openal.hpp"

#include "simple_vector.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <cstdio>
#include <cstring>
#include <iostream>

struct PhysicalDevice {
  ALCdevice *device;
  ALCcontext *context;
};
struct AudioDevice {
  simple_vector<PhysicalDevice> devices;
};

using namespace shambhala;
using namespace shambhala::audio;

static void list_audio_devices(const ALCchar *devices);
static AudioDevice device;

static PhysicalDevice createDevice(const ALCchar *device) {
  PhysicalDevice dev;
  dev.device = alcOpenDevice(device);
  dev.context = alcCreateContext(dev.device, NULL);
  return dev;
}

static void destroyDevice_S(int index) {
  alcMakeContextCurrent(NULL);
  alcDestroyContext(device.devices[index].context);
  alcCloseDevice(device.devices[index].device);
  device.devices.removeNShift(index);
}

void AudioOpenAL::destroyDevice() {
  for (int i = 0; i < device.devices.size(); i++) {
    destroyDevice_S(i);
  }
}
bool AudioOpenAL::initDevice() {
  device.devices.push(createDevice(NULL));
  return true;
}

void AudioOpenAL::configureListener(Listener listener) {
  alListener3f(AL_POSITION, listener.position.x, listener.position.y,
               listener.position.z);
  alListener3f(AL_VELOCITY, listener.velocity.x, listener.velocity.y,
               listener.velocity.z);
  alListenerfv(AL_ORIENTATION, (ALfloat *)&listener.orientation);
}

static void list_audio_devices(const ALCchar *devices) {
  const ALCchar *device = devices, *next = devices + 1;
  size_t len = 0;

  fprintf(stdout, "Devices list:\n");
  fprintf(stdout, "----------\n");
  while (device && *device != '\0' && next && *next != '\0') {
    fprintf(stdout, "%s\n", device);
    len = strlen((const char *)device);
    device += (len + 1);
    next += (len + 2);
  }
  fprintf(stdout, "----------\n");
}

void AudioOpenAL::loadwave(const char *name, ALenum *format, ALvoid *data,
                           ALsizei *size, ALsizei *freq, ALboolean *loop) {}
