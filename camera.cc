#include "camera.h"

#include "config.h"

double clamp(double v, double lo, double hi) {
  return v < lo ? lo : hi < v ? hi : v;
}

Camera::Camera() : ox_(0), oy_(0) {}

void Camera::update(const Player& player) {
  const double px = player.x();
  const double py = player.y();

  ox_ = px - kConfig.graphics.width / 2;
  oy_ = py - kConfig.graphics.height / 2;
}

int Camera::xoffset() const {
  return (int)ox_;
}

int Camera::yoffset() const {
  return (int)oy_;
}
