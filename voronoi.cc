#include "voronoi.h"

Voronoi::Voronoi() {}

float Voronoi::Site::distance(Voronoi::Site other) const {
  const float dx = x - other.x;
  const float dy = y - other.y;
  return dx * dx + dy * dy;
}

uint32_t Voronoi::Site::color() const {
  // const uint32_t r = (uint32_t)(x * 981011 + y * 282703) % 256;
  const uint32_t g = (uint32_t)(x * 336871 + y * 632351) % 256;
  // const uint32_t b = (uint32_t)(x * 390763 + y * 349913) % 256;

  const uint32_t b = (uint32_t)(y / 88.0f * 256);
  const uint32_t r = (uint32_t)(x);

  return r << 24 | g << 16 | b << 8 | 0xff;
}

void Voronoi::reset() {
  sites_.clear();
}

void Voronoi::add_point(float x, float y) {
  sites_.push_back({x, y});
}

Voronoi::Site Voronoi::get_site(float x, float y) const {
  if (sites_.empty()) return {0, 0};

  // TODO improve this
  Site nearest = sites_[0];
  float min_dist = sites_[0].distance({x, y});
  for (const auto& site : sites_) {
    const float dist = site.distance({x, y});
    if (dist < min_dist) {
      min_dist = dist;
      nearest = site;
    }
  }

  return nearest;
}
