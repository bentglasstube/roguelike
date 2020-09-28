#include "overworld.h"

#include <assert.h>

Overworld::Overworld(size_t width, size_t height) :
  width_(width), height_(height)
{
  assert(width * height < kMaxIndex);
}

void Overworld::generate(uint32_t seed) {
  rng_.seed(seed);

  // reset
  for (int y = 0; y < (int)height_; ++y) {
    for (int x = 0; x < (int)width_; ++x) {
      set_tile(x, y, Tile::Grass);
    }
  }

  // add points
  std::uniform_int_distribution<int> rx(0, width_);
  std::uniform_int_distribution<int> ry(0, height_);

  for (size_t i = 0; i < kNumPoints; ++i) {
    voronoi_.add_point(rx(rng_), ry(rng_));
  }
  voronoi_.generate();
  for (size_t i = 0; i < kRelaxRounds; ++i) {
    voronoi_.relax();
  }

  // generate maps

  // rasterize
}

void Overworld::draw(Graphics& graphics) const {
#ifndef NDEBUG
  voronoi_.draw_cell_borders(graphics);
#endif

  for (int y = 0; y < (int)height_; ++y) {
    for (int x = 0; x < (int)width_; ++x) {
      // TODO draw tile
    }
  }
}

void Overworld::set_tile(int x, int y, Overworld::Tile tile) {
  assert(x >= 0 && x < (int)width_);
  assert(y >= 0 && y < (int)height_);
  cells_[width_ * y + x] = tile;
}

Overworld::Tile Overworld::get_tile(int x, int y) const {
  if (x < 0 || x >= (int)width_) return boundary_tile();
  if (y < 0 || y >= (int)height_) return boundary_tile();
  return cells_[width_ * y + x];
}

Overworld::Tile Overworld::boundary_tile() const {
  return Tile::Water;
}
