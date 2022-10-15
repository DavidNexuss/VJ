#pragma once
#include <filesystem>
#include <string>
using uint8_t = unsigned char;
struct io_buffer {
  uint8_t *data;
  int length = 0;
};

struct IResource {
  virtual io_buffer *read() = 0;
  virtual void write() {}

  std::string resourcename;
  bool needsUpdate = true;
  bool readOnly = false;
  int useCount = 0;
};
