#pragma once
#include <core/core.hpp>
#include <string>
#include <unordered_map>
#include <vector>


namespace shambhala {

struct MemoryResource : public IResource {
  io_buffer buffer;
  virtual io_buffer *read() override;
  virtual void write() override;
#ifdef RELOAD_IO
  std::filesystem::file_time_type time;
#endif
};
struct IIO {
  std::vector<const char *> translators;

  std::string findFile(const std::string &path);
  MemoryResource *readFile(const std::string &path);
  void writeFile(MemoryResource *resource);
  void freeFile(MemoryResource *resource);

  virtual void filewatchMonitor();

private:
  static void addWatch(const std::string &path, MemoryResource *resource);
  std::unordered_map<std::string, MemoryResource> cachedBuffers;

  void insertFile(const std::string &name, MemoryResource resource);
  void eraseFile(const std::string &name);

protected:
  virtual void internal_writeFile(const std::string &path, io_buffer buffer);
  virtual io_buffer internal_readFile(const std::string &path) = 0;
  virtual void internal_freeFile(uint8_t *buffer) = 0;
};

} // namespace shambhala
