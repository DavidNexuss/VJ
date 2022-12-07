#pragma once
#include <component/video/model.hpp>
#include <component/video/texture.hpp>

namespace gi {
Texture *bakeAmbientOcclusion(std::vector<Model *> *modelList, int size,
                              int bounces);
}
