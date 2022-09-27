#include <shambhala.hpp>
namespace shambhala {
namespace resource {

IResource *createFromNullTerminatedString(const char *data,
                                          const char *resourcename);
MemoryResource *ioMemoryFile(const char *path);
TextureResource *stbiTextureMemory(MemoryResource *memoryResource);
TextureResource *stbiTextureFile(const char *path, int desiredChannels);
} // namespace resource
} // namespace shambhala
