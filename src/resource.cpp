#include "shambhala.hpp"
#include <stbimage/stb_image.h>
using namespace shambhala;

TextureResource *resource::stbiTextureFile(const char *path,
                                           int desired_channels) {
  TextureResource *resource = new TextureResource;
  resource->textureBuffer = stbi_load(path, &resource->width, &resource->height,
                                      &resource->components, desired_channels);
  return resource;
}
