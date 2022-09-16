#pragma once
using uint8_t = unsigned char;
struct io_buffer {
  uint8_t *data;
  int length;
};

namespace shambhala {
struct IIO {
  virtual io_buffer internal_readFile(const char *path) = 0;
  virtual void internal_freeFile(uint8_t *buffer) = 0;
};
} // namespace shambhala
