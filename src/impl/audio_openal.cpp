#include "audio_openal.hpp"
#include "simple_vector.hpp"
#include <AL/al.h>
#include <AL/alc.h>
// #include <AL/alut.h>
#include <adapters/log.hpp>
#include <core/core.hpp>
#include <cstdio>
#include <cstring>
#include <iostream>

struct PhysicalDevice {
  ALCdevice *device = nullptr;
  ALCcontext *context = nullptr;
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
  HardCheck(dev.device != nullptr);
  dev.context = alcCreateContext(dev.device, NULL);
  HardCheck(alcMakeContextCurrent(dev.context));

  LOGS(2, "[AL] al device created\n", 0);

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
    // destroyDevice_S(i);
  }
}
bool AudioOpenAL::initDevice() {
  // device.devices.push(createDevice(NULL));
  // alutInitWithoutContext(NULL, NULL);

  LOGS(2, "[AL] al device initialized\n", 0);
  return true;
}

static void list_audio_devices(const ALCchar *devices) {
  const ALCchar *device = devices, *next = devices + 1;
  size_t len = 0;

  while (device && *device != '\0' && next && *next != '\0') {
    len = strlen((const char *)device);
    device += (len + 1);
    next += (len + 2);
  }
}

void AudioOpenAL::loadwave(const char *name, ALenum *format, ALvoid **data,
                           ALsizei *size, ALfloat *freq, ALboolean *loop) {
  /*
  alutGetError();
  *data = alutLoadMemoryFromFile(name, format, size, freq);
  ALenum error = alutGetError();
  LOGS(2, "[ALUT] Loading from memory %s to %p for %d, errorState %d \n", name,
       *data, *size, error);

  if (error) {
    LOGS(2, "[ALUT] %s\n", alutGetErrorString(error));
  } */
}
