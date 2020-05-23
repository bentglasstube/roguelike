#include "overworld.h"

Overworld::Overworld(int width, int height) :
  width_(width), height_(height) {}

void Overworld::generate(uint32_t seed) {
  rng_.seed(seed);

  // reset
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      // set_tile(x, y, Tile::Grass);
    }
  }

  // add points
  std::uniform_int_distribution<int> rx(0, width_);
  std::uniform_int_distribution<int> ry(0, height_);

  for (size_t i = 0; i < kNumPoints; ++i) {
    voronoi_.add_point(rx(rng_), ry(rng_));
  }

  // voronoize ??

  // relax

  // generate maps

  // rasterize
}

void Overworld::draw(Graphics& graphics) const {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      const auto& site = voronoi_.get_site(x, y);
      graphics.draw_pixel(x, y, site.color());
    }
  }
}
