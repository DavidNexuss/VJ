#include <shambhala.hpp>
namespace shambhala {
namespace resource {
MemoryResource *ioMemoryFile(const char *path);
TextureResource *stbiTextureMemory(MemoryResource *memoryResource);
TextureResource *stbiTextureFile(const char *path, int desiredChannels);
} // namespace resource
} // namespace shambhala
