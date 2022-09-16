#include "resource.hpp"
#include <stbimage/stb_image.h>
using namespace shambhala;

TextureResource *resource::stbiTextureFile(const char *path,
                                           int desired_channels) {
  TextureResource *resource = new TextureResource;
  resource->textureBuffer = stbi_load(path, &resource->width, &resource->height,
                                      &resource->components, desired_channels);
  resource->resourcename = path;
  return resource;
}

MemoryResource *resource::ioMemoryFile(const char *path) {
  MemoryResource *resource = new MemoryResource;
  resource->buffer = io()->internal_readFile(path);
  resource->resourcename = path;
  return resource;
}

io_buffer TextureResource::read() { return {nullptr, 0}; }
io_buffer MemoryResource::read() { return buffer; }

bool IResource::claim() {
  bool ret = this->needsUpdate;
  this->needsUpdate = false;
  return ret;
}
