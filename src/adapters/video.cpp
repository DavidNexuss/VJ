#include "video.hpp"
using namespace shambhala;
using namespace shambhala::video;

video::BufferUploadDesc video::descUpload(simple_vector_span span,
                                          GLuint buffer) {
  BufferUploadDesc desc;
  desc.buffer = span.data;
  desc.size = span._size;
  desc.id = buffer;
  desc.start = 0;
  return desc;
}
video::TextureDesc video::descDepthTexture() {
  video::TextureDesc desc;
  desc.clamp = false;
  return desc;
}
video::TextureDesc video::descStencilTexture() {
  video::TextureDesc desc;
  desc.clamp = false;
  return desc;
}
video::TextureUploadDesc video::descDepthUpload(int width, int height,
                                                GLuint texture) {
  video::TextureUploadDesc desc;
  desc.textureID = texture;
  desc.isDepth = true;
  desc.width = width;
  desc.height = height;
  return desc;
}

video::TextureUploadDesc video::descStencilUpload(int width, int height,
                                                  GLuint texture) {
  video::TextureUploadDesc desc;
  desc.textureID = texture;
  desc.isStencil = true;
  desc.width = width;
  desc.height = height;
  return desc;
}

video::TextureFormat video::descTextureFormat(bool hdr, int components) {

  const static int internalFormatHDR[] = {};
  const static int internalFormat[] = {GL_R8, GL_RG8, GL_RGB8, GL_RGBA8};
  const static int externalFormat[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};

  video::TextureFormat format;
  format.externalFormat = externalFormat[components - 1];
  format.internalFormat = internalFormat[components - 1];

  format.type = hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;

  return format;
}
