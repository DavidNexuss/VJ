#pragma once
#include <core/core.hpp>
#include <string>
#include <unordered_map>
#include <vector>

struct MemoryResource : public IResource {
  io_buffer buffer;
  virtual io_buffer *read() override;
  virtual void write() override;
};

namespace io {

std::string findFile(const std::string &path);
MemoryResource *readFile(const std::string &path);
void writeFile(MemoryResource *resource);
void freeFile(MemoryResource *resource);

/** TO IMPLEMENT **/

io_buffer internal_readFile(const std::string &path);
void internal_freeFile(uint8_t *buffer);
} // namespace io
