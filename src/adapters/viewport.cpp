#include "viewport.hpp"

bool shambhala::IViewport::isKeyPressed(int keyCode) {
  return pressed[keyCode];
}

bool shambhala::IViewport::isKeyJustPressed(int keyCode) {
  return justPressed[keyCode];
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

bool shambhala::IViewport::isMousePressed() { return mousePressed; }

double shambhala::IViewport::aspectRatio() {
  return screenWidth / screenHeight;
}

glm::vec2 shambhala::IViewport::getMouseViewportCoords() {
  return glm::vec2(xpos / screenWidth, ypos / screenHeight);
}

bool shambhala::IViewport::isMiddleMousePressed() { return middleMousePressed; }
bool shambhala::IViewport::isRightMousePressed() { return rightMousePressed; }
