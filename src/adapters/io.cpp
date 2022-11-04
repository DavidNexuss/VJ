#include "io.hpp"
#include "shambhala.hpp"
#include <adapters/log.hpp>

using namespace shambhala;
void shambhala::IIO::freeFile(MemoryResource *resource) {
  resource->useCount--;
  if (resource->useCount < 1) {
    eraseFile(resource->resourcename);
  }
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
#ifdef RELOAD_IO
      resource.time = std::filesystem::last_write_time(path);
#endif
      insertFile(path, resource);
      auto it = cachedBuffers.find(path);
      return &it->second;
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
void shambhala::IIO::writeFile(MemoryResource *resource) {
  LOG("[IO] Writing resource %s to disk\n", resource->resourcename.c_str());
  internal_writeFile(resource->resourcename, resource->buffer);
}

void shambhala::IIO::eraseFile(const std::string &name) {
  cachedBuffers.erase(name);
}
void shambhala::IIO::insertFile(const std::string &name,
                                MemoryResource resource) {

  auto it = cachedBuffers.insert({name, resource});
  addWatch(name, &it.first->second);
}

void shambhala::IIO::addWatch(const std::string &name,
                              MemoryResource *resource) {}

#include <fstream>
void shambhala::IIO::internal_writeFile(const std::string &name,
                                        io_buffer buffer) {
  std::ofstream file(name);
  file.write((const char *)buffer.data, buffer.length);
  file.close();
}
//------------------[FILE WATCHER]
#include <filesystem>
void shambhala::IIO::filewatchMonitor() {
#ifdef RELOAD_IO
  for (auto &it : cachedBuffers) {
    auto last_write_time = std::filesystem::last_write_time(it.first);
    if (last_write_time > it.second.time) {
      MemoryResource *resource = &it.second;
      delete[] resource->buffer.data;
      resource->buffer = internal_readFile(resource->resourcename);
      resource->signalUpdate();
      resource->time = last_write_time;
    }
  }
#endif
}

//------------------[MEMORY RESOURCE]

void MemoryResource::write() { shambhala::io()->writeFile(this); }
