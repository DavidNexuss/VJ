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
  std::string resourcename;
  bool needsUpdate = true;
  int useCount = 0;
  std::filesystem::file_time_type time;
};

struct MemoryResource : public IResource {
  io_buffer buffer;
  virtual io_buffer *read() override;
};
