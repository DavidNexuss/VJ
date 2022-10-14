#include "io.hpp"
#include <adapters/log.hpp>

using namespace shambhala;
void shambhala::IIO::freeFile(MemoryResource *resource) {
  resource->useCount--;
  if (resource->useCount < 1)
    cachedBuffers.erase(resource->resourcename);
}

// TODO: Fix something
std::string shambhala::IIO::findFile(const std::string &path) {}

MemoryResource *shambhala::IIO::readFile(const std::string &path) {
  LOG("Reading %s:", path.c_str());
  auto it = cachedBuffers.find(path);
  if (it != cachedBuffers.end()) {
    it->second.useCount++;
    return &it->second;
  }

  auto read = [&](const std::string &path) -> MemoryResource * {
    MemoryResource resource;
    resource.buffer = internal_readFile(path);
    resource.resourcename = path;
    if (resource.buffer.length > 0) {
      resource.useCount++;
      auto it = cachedBuffers.insert({path, resource});
      return &it.first->second;
    }
    return nullptr;
  };

  auto resolvePath = [&](const std::string &path, const char *translator) {
    static char buffer[4096] = {0};
    int n = sprintf(buffer, translator, path.c_str());
    return std::string(buffer, n);
  };

  for (int i = 0; i < translators.size(); i++) {
    std::string localPath = resolvePath(path, translators[i]);
    MemoryResource *result;
    if ((result = read(localPath.c_str())) != nullptr) {
      return result;
    }
  }

  MemoryResource *buff = read(path);
  if (buff == nullptr) {
    LOG("[IO] %s not found!", path.c_str());
  }

  return buff;
}
