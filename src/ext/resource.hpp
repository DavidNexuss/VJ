#include <shambhala.hpp>
namespace shambhala {
namespace resource {

IResource *createFromNullTerminatedString(const char *data,
                                          const std::string &name);
MemoryResource *ioMemoryFile(const std::string &path);
TextureResource *stbiTextureMemory(MemoryResource *memoryResource);
TextureResource *stbiTextureFile(const std::string &path, int desiredChannels);
} // namespace resource
} // namespace shambhala
