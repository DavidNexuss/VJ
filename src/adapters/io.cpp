#include "io.hpp"
#include <adapters/log.hpp>
void shambhala::IIO::freeFile(io_buffer *buffer) {
  buffer->useCount--;
  if (buffer->useCount < 1)
    cachedBuffers.erase(buffer->resourcename);
}

// TODO: Fix something
std::string shambhala::IIO::findFile(const char *path) {

  for (int i = 0; i < translators.size(); i++) {
  }
}
io_buffer *shambhala::IIO::readFile(const char *path) {
  LOG("Reading %s:", path);
  auto it = cachedBuffers.find(path);
  if (it != cachedBuffers.end()) {
    it->second.useCount++;
    return &it->second;
  }

  auto read = [&](const char *path) -> io_buffer * {
    io_buffer buffer = internal_readFile(path);
    if (buffer.length > 0) {
      buffer.useCount++;
      auto it = cachedBuffers.insert({path, buffer});
      return &it.first->second;
    }
    return nullptr;
  };

  auto resolvePath = [&](const char *path, const char *translator) {
    static char buffer[4096] = {0};
    int n = sprintf(buffer, translator, path);
    return std::string(buffer, n);
  };

  for (int i = 0; i < translators.size(); i++) {
    std::string localPath = resolvePath(path, translators[i]);
    io_buffer *result;
    if ((result = read(localPath.c_str())) != nullptr) {
      return result;
    }
  }

  io_buffer *buff = read(path);
  if (buff == nullptr) {
    LOG("[IO] %s not found!", path);
  }

  return buff;
}
