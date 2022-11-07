#pragma once
#include "entity/player.hpp"
#include "ext.hpp"
#include "font.hpp"
#include "shambhala.hpp"

namespace joc {
extern BitMapFont *font;
extern Player *player;
extern float *globalTime;
void renderBox(glm::vec2 start, glm::vec2 end, glm::vec4 color);

extern shambhala::worldmats::Clock *clock;
} // namespace joc
