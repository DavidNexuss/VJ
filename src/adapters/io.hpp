#pragma once
#include <string>
#include <unordered_map>
#include <vector>

using uint8_t = unsigned char;
struct io_buffer {
  uint8_t *data;
  int length = 0;
  int useCount;
  const char *resourcename;
};

namespace shambhala {

struct IIO {
  std::vector<const char *> translators;
  io_buffer *readFile(const char *path);
  void freeFile(io_buffer *buffer);

  std::string findFile(const char *path);
  virtual io_buffer internal_readFile(const char *path) = 0;
  virtual void internal_freeFile(uint8_t *buffer) = 0;

private:
  std::unordered_map<const char *, io_buffer> cachedBuffers;
};

} // namespace shambhala
