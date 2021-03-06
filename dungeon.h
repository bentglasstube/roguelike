#pragma once

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "graphics.h"
#include "spritemap.h"

#include "rect.h"

// I have made a huge fucking mess of circular dependencies so I need to
// forward declare the entity class to get things to build.
class Entity;

class Dungeon {
  public:

    struct TuningParams {
      double room_density;
      double straightness;
      double extra_doors;
      int sections;
    };

    enum class Tile {
      OutOfBounds, Wall, Hallway, Room,
      DoorLocked, DoorClosed, DoorOpen,
      StairsUp, StairsDown,
      ChestClosed, ChestOpen,
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

    void generate(unsigned int seed);

    Position grid_coords(double px, double py) const;

    void reveal();
    void hide();
    void calculate_visibility(int x, int y);

    const Cell& get_cell(int x, int y) const;
    Position find_tile(Tile tile) const;
    bool any_entity_at(int x, int y) const;
    bool any_entity_at(int x, int y, std::function<bool(const std::unique_ptr<Entity>&)> pred) const;

    void update(Entity& player, unsigned int elapsed);
    void draw(Graphics& graphics, int hud_height, int xo, int yo) const;
    void draw_map(Graphics& graphics, const Rect& source, const Rect& dest) const;

    void add_drop(double x, double y);

    bool walkable(int x, int y) const;
    bool transparent(int x, int y) const;

    bool box_walkable(const Rect& r) const;

    void open_door(int x, int y);
    void close_door(int x, int y);
    void open_chest(int x, int y);

  private:
    static constexpr int kTileSize = 16;
    static constexpr int kHalfTile = kTileSize / 2;
    static constexpr int kMaxVisibility = 9;
    static constexpr Cell kBadCell = { Tile::OutOfBounds, 0, false, false };

    enum class Direction { North, South, East, West };

    struct Connector {
      int x, y, region;
    };

    struct Shadow {
      double start, end;
      bool contains(const Shadow& other) const;
    };

    class ShadowLine {
      public:
        ShadowLine();
        bool is_shadowed(const Shadow& shadow) const;
        void add(const Shadow& shadow);

      private:
        std::vector<Shadow> shadows_;
    };

    int width_, height_;
    TuningParams params_;
    std::default_random_engine rand_, rng_;
    Cell cells_[1024][1024];
    std::vector<std::unique_ptr<Entity>> entities_;

    SpriteMap tiles_;

    void set_tile(int x, int y, Tile tile);
    void set_region(int x, int y, int region);
    void set_visible(int x, int y, bool visible);

    Position find_open_space() const;

    int random_odd(int min, int max);
    int place_room(int region);
    int is_connector(int x, int y, int region) const;
    void replace_region(int from, int to);
    void place_key();

    int adjacent_count(int x, int y, Tile tile) const;
    bool is_dead_end(int x, int y) const;

    bool box_visible(const Rect& r) const;

    int get_cell_color(int x, int y) const;
    std::vector<Connector> get_connectors(int region, int min) const;
};
