#pragma once
#include <adapters/io.hpp>
#include <adapters/video.hpp>

struct ITexture {
  virtual GLuint gl() = 0;
  virtual GLenum getMode() { return GL_TEXTURE_2D; }
};

struct TextureResource : public IResource {
  uint8_t *textureBuffer = nullptr;
  int width;
  int height;
  int components = 3;
  bool hdrSpace = false;
  virtual io_buffer *read() override;
};

struct Texture : public ITexture, video::TextureDesc {

  bool needsUpdate();
  void addTextureResource(TextureResource *textureData);

  GLuint gl() override;
  GLenum getMode() override;

  std::vector<ResourceHandlerAbstract<TextureResource>> textureData;

private:
  GLuint gl_textureID = -1;
};

namespace resource {

TextureResource *stbiTextureMemory(MemoryResource *memoryResource);
TextureResource *stbiTextureFile(const std::string &path, int desiredChannels);
} // namespace resource
