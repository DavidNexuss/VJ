#include "video.hpp"
using namespace shambhala;
using namespace shambhala::video;
video::BufferUploadDesc shambhala::descUpload(simple_vector_span span,
                                              GLuint buffer) {
  BufferUploadDesc desc;
  desc.buffer = span.data;
  desc.size = span._size;
  desc.id = buffer;
  desc.start = 0;
  return desc;
}
video::TextureDesc shambhala::descDepthTexture() {
  video::TextureDesc desc;
  desc.clamp = false;
  desc.cubemap = false;
  desc.useNeareast = false;
  return desc;
}
video::TextureDesc shambhala::descStencilTexture() {
  video::TextureDesc desc;
  desc.clamp = false;
  desc.cubemap = false;
  desc.useNeareast = false;
  return desc;
}
video::TextureUploadDesc shambhala::descDepthUpload(int width, int height,
                                                    GLuint texture) {
  video::TextureUploadDesc desc;
  desc.textureID = texture;
  desc.isDepth = true;
  desc.width = width;
  desc.height = height;
  return desc;
}

video::TextureUploadDesc shambhala::descStencilUpload(int width, int height,
                                                      GLuint texture) {
  video::TextureUploadDesc desc;
  desc.textureID = texture;
  desc.isStencil = true;
  desc.width = width;
  desc.height = height;
  return desc;
}

video::TextureFormat shambhala::descTextureFormat(bool hdr, int components) {

  const static int internalFormatHDR[] = {};
  const static int internalFormat[] = {GL_RED, GL_RG8, GL_RGB8, GL_RGBA8};
  const static int externalFormat[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};

  video::TextureFormat format;
  format.externalFormat = externalFormat[components - 1];
  format.internalFormat = internalFormat[components - 1];

  format.type = hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;

  return format;
}