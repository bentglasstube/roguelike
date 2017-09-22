#pragma once

#include <random>

#include "graphics.h"
#include "spritemap.h"

class Dungeon {
  public:

    struct TuningParams {
      double room_density;
      double straightness;
      double extra_doors;
    };

    enum class Tile {
      OutOfBounds, Wall, Hallway, Room,
      DoorClosed, DoorOpen, StairsUp, StairsDown,
    };

    struct Cell {
      Tile tile;
      int region;
      bool visible;
      bool seen;
    };

    struct Position {
      int x, y;
    };

    Dungeon(int width, int height, TuningParams params);

    void generate();
    void generate(unsigned int seed);

    std::pair<int,int> grid_coords(int px, int py) const;

    void reveal();
    void calculate_visibility(int x, int y);

    Cell get_cell(int x, int y) const;
    Position find_tile(Tile tile) const;

    void draw(Graphics& graphics, int xo, int yo) const;

    bool walkable(int x, int y) const;
    bool interact(int x, int y);

  private:
    static constexpr int kTileSize = 16;

    enum class Direction { North, South, East, West };

    struct Connector {
      int x, y, region;
    };

    int width_, height_;
    TuningParams params_;
    std::default_random_engine rand_;
    Cell cells_[1024][1024];

    SpriteMap tiles_;

    void set_tile(int x, int y, Tile tile);
    void set_region(int x, int y, int region);
    void set_visible(int x, int y, bool visible);

    Position find_open_space() const;

    int random_odd(int min, int max);
    int place_room(int region);
    int is_connector(int x, int y) const;
    void replace_region(int from, int to);

    int adjacent_count(int x, int y, Tile tile) const;
    bool is_dead_end(int x, int y) const;
};
