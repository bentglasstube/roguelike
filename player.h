#pragma once

#include "graphics.h"
#include "spritemap.h"

#include "dungeon.h"

class Player {
  public:

    enum class Direction { North, South, East, West };

    Player(int x, int y);

    double x() const;
    double y() const;
    void set_pos(double x, double y);

    void move(Direction direction);
    void stop();
    bool interact(Dungeon& dungeon);
    void attack();

    void update(const Dungeon& dungeon, unsigned int elapsed);
    void draw(Graphics& graphics, int xo, int yo) const;

  private:

    enum class State { Standing, Walking, Attacking };

    static constexpr double kSpeed = 0.1;
    static constexpr int kAttackTime = 250;
    static constexpr int kTileSize = 16;
    static constexpr int kHalfTile = kTileSize / 2;

    SpriteMap sprites_;
    double x_, y_;
    Direction facing_;
    int timer_;
    State state_;

    int frame() const;
};
