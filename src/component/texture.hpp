#pragma once
#include "component.hpp"
struct ITexture {
  virtual SHuint gl() = 0;
  virtual SHenum getMode() { return GL_TEXTURE_2D; }
};
