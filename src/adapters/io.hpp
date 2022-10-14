#pragma once
#include <core/core.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace shambhala {

struct IIO {
  std::vector<const char *> translators;

  std::string findFile(const std::string &path);
  MemoryResource *readFile(const std::string &path);
  void freeFile(MemoryResource *resource);

private:
  static void addWatch(const std::string &path, MemoryResource *resource);
  std::unordered_map<std::string, MemoryResource> cachedBuffers;

protected:
  virtual io_buffer internal_readFile(const std::string &path) = 0;
  virtual void internal_freeFile(uint8_t *buffer) = 0;
};

} // namespace shambhala
