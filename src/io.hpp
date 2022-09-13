#pragma once
using uint8_t = unsigned char;
struct io_buffer {
  uint8_t *data;
  int length;
};

namespace shambhala {
struct IIO {
  virtual io_buffer readFile(const char *path);
};
} // namespace shambhala
