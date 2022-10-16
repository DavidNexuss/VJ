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
