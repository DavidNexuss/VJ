#include "viewport.hpp"
#include <stdio.h>
#include <unordered_map>
#include <vector>
using namespace shambhala;

static std::unordered_map<unsigned int, bool> pressed;
static bool inputEnabled;
static int currentFrame;
static bool mousePressed;
static std::vector<glm::vec2> viewports;
static int xpos;
static int ypos;
static bool middlePressed;
static bool rightPressed;
static float scrollX;
static float scrollY;
static int screenWidth;
static int screenHeight;

bool viewport::isKeyPressed(int keyCode) {
  return pressed[keyCode] && inputEnabled;
}

bool viewport::isKeyJustPressed(int keyCode) {
  int difference = currentFrame - pressed[keyCode];
  return (difference < 2) && inputEnabled;
}

void viewport::pushViewport(int width, int height) {}

void viewport::popViewport() {}

bool viewport::isMousePressed() { return mousePressed; }

float viewport::aspectRatio() {
  return viewport::getScreenWidth() / viewport::getScreenHeight();
}

glm::vec2 viewport::getMouseViewportCoords() {
  return glm::vec2{xpos / getScreenWidth(), ypos / getScreenHeight()};
}

bool viewport::isMiddleMousePressed() { return middlePressed; }
bool viewport::isRightMousePressed() { return rightPressed; }

void viewport::enableInput(bool enable) { inputEnabled = enable; }

bool viewport::isInputEnabled() { return inputEnabled; }

void viewport::setKeyPressed(int keycode, bool active) {
  if (active) {
    pressed[keycode] = currentFrame;
  } else {
    pressed[keycode] = 0;
  }
}

float viewport::getScrolllX() {
  if (isInputEnabled())
    return scrollX;
  return 0.0f;
}

float viewport::getScrolllY() {
  if (isInputEnabled())
    return scrollY;
  return 0.0f;
}

float viewport::getScreenWidth() { return screenWidth; }
float viewport::getScreenHeight() { return screenHeight; }

float viewport::getWidth() {
  if (viewports.empty())
    return screenWidth;
  return viewports.back().x;
}
float viewport::getHeight() {
  if (viewports.empty())
    return screenHeight;
  return viewports.back().y;
}

float viewport::getX() { return xpos; }
float viewport::getY() { return ypos; }

void viewport::setX(float x) { xpos = x; }
void viewport::setY(float y) { ypos = y; }

void viewport::setScreenWidth(float x) { screenWidth = x; }
void viewport::setScreenHeight(float y) { screenHeight = y; }

void viewport::setScrollX(float x) { scrollX = x; }
void viewport::setScrollY(float y) { scrollY = y; }
