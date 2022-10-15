#include "resource.hpp"
#include <standard.hpp>
#include <stbimage/stb_image.h>
using namespace shambhala;

static io_buffer nullResource = {nullptr, 0};

TextureResource *resource::stbiTextureFile(const std::string &path,
                                           int desired_channels) {
  TextureResource *resource = new TextureResource;
  MemoryResource *memresource = io()->readFile(path);
  io_buffer *buff = memresource->read();
  stbi_info_from_memory(buff->data, buff->length, &resource->width,
                        &resource->height, &resource->components);

  resource->textureBuffer = stbi_load_from_memory(
      buff->data, buff->length, &resource->width, &resource->height,
      &resource->components, resource->components);

  resource->resourcename = path;

  io()->freeFile(memresource);
  return resource;
}

MemoryResource *resource::ioMemoryFile(const std::string &path) {
  return io()->readFile(path);
}

io_buffer *TextureResource::read() { return &nullResource; }
io_buffer *MemoryResource::read() { return &buffer; }

IResource *shambhala::resource::createFromNullTerminatedString(
    const char *data, const std::string &resourcename) {
  MemoryResource *resource = new MemoryResource;
  resource->buffer = {(uint8_t *)data, Standard::resourceNullTerminated};
  resource->resourcename = resourcename;
  resource->readOnly = true;
  return resource;
}
