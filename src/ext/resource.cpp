#include "resource.hpp"
#include <stbimage/stb_image.h>
using namespace shambhala;

static io_buffer nullResource = {nullptr, 0};

TextureResource *resource::stbiTextureFile(const char *path,
                                           int desired_channels) {
  TextureResource *resource = new TextureResource;
  io_buffer *buff = io()->readFile(path);

  stbi_info_from_memory(buff->data, buff->length, &resource->width,
                        &resource->height, &resource->components);

  resource->textureBuffer = stbi_load_from_memory(
      buff->data, buff->length, &resource->width, &resource->height,
      &resource->components, resource->components);

  resource->resourcename = path;
  return resource;
}

MemoryResource *resource::ioMemoryFile(const char *path) {
  MemoryResource *resource = new MemoryResource;
  resource->buffer = io()->readFile(path);
  resource->resourcename = path;
  return resource;
}

io_buffer *TextureResource::read() { return &nullResource; }
io_buffer *MemoryResource::read() { return buffer; }

bool IResource::claim() {
  bool ret = this->_needsUpdate;
  this->_needsUpdate = false;
  return ret;
}

bool IResource::needsUpdate() { return this->_needsUpdate; }
