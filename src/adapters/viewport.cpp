#include "viewport.hpp"
#include <stdio.h>
bool shambhala::IViewport::isKeyPressed(int keyCode) {
  return pressed[keyCode] && inputEnabled;
}

bool shambhala::IViewport::isKeyJustPressed(int keyCode) {
  int difference = currentFrame - pressed[keyCode];
  return (difference < 2) && inputEnabled;
}

void shambhala::IViewport::fakeViewportSize(int width, int height) {
  backedWidth = screenWidth;
  backedHeight = screenHeight;
  screenWidth = width == 0 ? screenWidth : width;
  screenHeight = height == 0 ? screenHeight : height;
  realScreenWidth = width;
  realScreenHeight = height;
}

void shambhala::IViewport::restoreViewport() {
  screenWidth = backedWidth;
  screenHeight = backedHeight;
}

bool shambhala::IViewport::isMousePressed() {
  return inputEnabled && mousePressed;
}

double shambhala::IViewport::aspectRatio() {
  return screenWidth / screenHeight;
}

glm::vec2 shambhala::IViewport::getMouseViewportCoords() {
  return glm::vec2(xpos / screenWidth, ypos / screenHeight);
}

bool shambhala::IViewport::isMiddleMousePressed() { return middleMousePressed; }
bool shambhala::IViewport::isRightMousePressed() { return rightMousePressed; }

void shambhala::IViewport::enableInput(bool enable) { inputEnabled = enable; }

bool shambhala::IViewport::isInputEnabled() { return inputEnabled; }

void shambhala::IViewport::setKeyPressed(int keycode, bool active) {
  if (active) {
    pressed[keycode] = currentFrame;
  } else {
    pressed[keycode] = 0;
  }
}

void shambhala::IViewport::notifyFrame(int frame) { currentFrame = frame; }
void shambhala::IViewport::notifyFrameEnd() {
  scrollX = 0;
  scrollY = 0;
}

float shambhala::IViewport::getScrolllX() {
  if (isInputEnabled())
    return scrollX;
  return 0.0f;
}

float shambhala::IViewport::getScrolllY() {
  if (isInputEnabled())
    return scrollY;
  return 0.0f;
}

float shambhala::IViewport::getScreenWidth() {
  if (screenWidth == -1)
    return realScreenWidth;
  return screenWidth;
}
float shambhala::IViewport::getScreenHeight() {
  if (screenHeight == -1)
    return realScreenHeight;
  return screenHeight;
}

float shambhala::IViewport::getX() { return xpos; }
float shambhala::IViewport::getY() { return ypos; }

void shambhala::IViewport::setX(float x) { xpos = x; }
void shambhala::IViewport::setY(float y) { ypos = y; }

void shambhala::IViewport::setWidth(float x) { screenWidth = x; }
void shambhala::IViewport::setHeight(float y) { screenHeight = y; }

void shambhala::IViewport::setScrollX(float x) { scrollX = x; }
void shambhala::IViewport::setScrollY(float y) { scrollY = y; }
