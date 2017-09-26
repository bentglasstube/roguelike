#include "player.h"

#include <iostream>

Player::Player(int x, int y) :
  sprites_("player.png", 4, kTileSize, kTileSize),
  x_(x), y_(y),
  facing_(Direction::South),
  timer_(0),
  state_(State::Standing) {}

double Player::x() const {
  return x_;
}

double Player::y() const {
  return y_;
}

void Player::set_pos(double x, double y) {
  x_ = x;
  y_ = y;
}

void Player::move(Player::Direction direction) {
  if (state_ != State::Attacking) {
    facing_ = direction;
    state_ = State::Walking;
  }
}

void Player::stop() {
  if (state_ == State::Walking) {
    state_ = State::Standing;
  }
}

bool Player::interact(Dungeon& dungeon) {
  int cx = x_ / 16;
  int cy = (y_ - 1) / 16;

  switch (facing_) {
    case Direction::North: --cy; break;
    case Direction::South: ++cy; break;
    case Direction::West: --cx; break;
    case Direction::East: ++cx; break;
  }

  return dungeon.interact(cx, cy);
}

void Player::attack() {
  state_ = State::Attacking;
  timer_ = 0;
}

// Helper to lock walking to half-tile grid
std::pair<double, double> grid_walk(double delta, double minor, int grid) {
  const double dmin = minor - 8 * (int)(minor / 8);
  if (dmin > grid) {
    if (delta > 8 - dmin) {
      return { delta - 8 + dmin, 8 - dmin };
    } else {
      return { 0, delta };
    }
  } else {
    if (delta > dmin) {
      return { delta - dmin, -dmin };
    } else {
      return { 0, -delta };
    }
  }
}

void Player::update(const Dungeon& dungeon, unsigned int elapsed) {
  if (state_ == State::Walking) {
    const double delta = kSpeed * elapsed;
    std::pair<double, double> d;
    double dx = 0, dy = 0;

    switch (facing_) {
      case Direction::North:
      case Direction::South:
        d = grid_walk(delta, x_, kHalfTile / 2);
        dx = d.second;
        dy = d.first * (facing_ == Direction::North ? -1 : 1);
        break;

      case Direction::West:
      case Direction::East:
        d = grid_walk(delta, y_, kHalfTile / 2);
        dx = d.first * (facing_ == Direction::West ? -1 : 1);
        dy = d.second;
        break;
    }

    auto c1 = dungeon.grid_coords(x_ + dx - kHalfTile + 1, y_ + dy - kHalfTile + 1);
    auto c2 = dungeon.grid_coords(x_ + dx + kHalfTile - 1, y_ + dy - kHalfTile + 1);
    auto c3 = dungeon.grid_coords(x_ + dx - kHalfTile + 1, y_ + dy);
    auto c4 = dungeon.grid_coords(x_ + dx + kHalfTile - 1, y_ + dy);

    if (!dungeon.walkable(c1.first, c1.second)) return;
    if (!dungeon.walkable(c2.first, c2.second)) return;
    if (!dungeon.walkable(c3.first, c3.second)) return;
    if (!dungeon.walkable(c4.first, c4.second)) return;

    timer_ = (timer_ + elapsed) % 1000;
    x_ += dx;
    y_ += dy;
  } else if (state_ == State::Attacking) {
    timer_ += elapsed;
    if (timer_ > kAttackTime) {
      state_ = State::Standing;
      timer_ = 0;
    }
  }
}

void Player::draw(Graphics& graphics, int xo, int yo) const {
  const int x = x_ - kHalfTile - xo;
  const int y = y_ - kTileSize - yo;
  int f = timer_ / 250;

  switch (facing_) {
    case Direction::East:
    case Direction::West:
      f += 4; break;
    case Direction::South:
      f += 8; break;
    default:
      // do nothing
      break;
  }

  sprites_.draw_ex(graphics, frame(), x, y, facing_ == Direction::West, 0, 0, 0);

  /* if (state_ == State::Attacking) { */
  /*   int weapon_sprite = 0; */
  /*   switch (facing_) { */
  /*   } */
  /* } */

#ifndef NDEBUG
  const SDL_Rect body = { x, y + kHalfTile, kTileSize, kHalfTile };
  graphics.draw_rect(&body, 0xff0000ff, false);
#endif

}

int Player::frame() const {
  int d = 0;

  switch (facing_) {
    case Direction::North: d = 0; break;
    case Direction::South: d = 2; break;
    default: d = 1; break;
  }

  switch (state_) {
    case State::Attacking:
      return 12 + d;

    case State::Standing:
    case State::Walking:
      return d * 4 + timer_ / 250;

    default:
      return 0;
  }
}
