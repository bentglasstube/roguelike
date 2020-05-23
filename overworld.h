#pragma once

#include <random>

#include "graphics.h"

#include "voronoi.h"

class Overworld {
  public:

    enum class Tile { Grass, Water, Sand, Tree, Rock };

    Overworld(int width, int height);

    void set_tile(int x, int y, Tile tile);
    Tile get_tile(int x, int y) const;

    void draw(Graphics& graphics) const;
    void generate(uint32_t seed);

  private:

    static constexpr size_t kNumPoints = 1024;

    struct Cell {
      int x, y;
      float height, moisture;
    };

    int width_, height_;
    std::array<Tile, 4096> cells_;
    std::mt19937 rng_;

    Voronoi voronoi_;
};
