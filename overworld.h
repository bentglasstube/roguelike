#pragma once

#include <random>

#include "graphics.h"

#include "stb_perlin.h"
#include "voronoi.h"

class Overworld {
  public:

    enum class Tile { Grass, Water, Sand, Tree, Rock };

    Overworld(size_t width, size_t height);

    void set_tile(int x, int y, Tile tile);
    Tile get_tile(int x, int y) const;

    void draw(Graphics& graphics) const;
    void generate(uint32_t seed);

  private:

    static constexpr size_t kNumPoints = 512;
    static constexpr size_t kRelaxRounds = 20;
    static constexpr size_t kMaxIndex = 65536;

    struct Cell {
      int x, y;
      float height, moisture;
    };

    size_t width_, height_;
    std::array<Tile, kMaxIndex> cells_;
    std::mt19937 rng_;

    Voronoi voronoi_;

    Tile boundary_tile() const;
};
