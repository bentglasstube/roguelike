#include "spike_trap.h"

#include <iostream>

SpikeTrap::SpikeTrap(double x, double y) :
  Entity("enemies.png", 8, x, y),
  state_(State::Waiting) {}

void SpikeTrap::ai(const Dungeon& dungeon, const Entity& player) {
  if (state_ != State::Waiting) return;

  const auto p = dungeon.grid_coords(player.x(), player.y());
  const auto s = dungeon.grid_coords(x(), y());

  if (p.first == s.first) {
    const bool north = p.second < s.second;
    const int start = north ? p.second : s.second;
    const int end = north ? s.second : p.second;

    for (int y = start + 1; y < end; ++y) {
      if (!dungeon.walkable(p.first, y)) return;
    }

    facing_ = north ? Direction::North : Direction::South;
    state_ = State::Charging;
  }

  if (p.second == s.second) {
    const bool west = p.second < s.second;
    const int start = west ? p.second : s.second;
    const int end = west ? s.second : p.second;

    for (int x = start + 1; x < end; ++x) {
      if (!dungeon.walkable(x, p.second)) return;
    }

    facing_ = west ? Direction::West : Direction::East;
    state_ = State::Charging;
  }
}

void SpikeTrap::update(const Dungeon& dungeon, unsigned int elapsed) {
  if (state_ == State::Waiting) return;

  if (state_ == State::Hold) {
    timer_ += elapsed;
    if (timer_ > kHoldTime) {
      state_ = State::Retreating;
      timer_ = 0;
      facing_ = Entity::reverse_direction(facing_);
    }
  }

  double speed = state_ == State::Charging ? kChargingSpeed : kRetreatingSpeed;
  double dx = 0, dy = 0;

  switch (facing_) {
    case Direction::North:
      dy = -speed * elapsed;
      break;

    case Direction::South:
      dy = speed * elapsed;
      break;

    case Direction::East:
      dx = speed * elapsed;
      break;

    case Direction::West:
      dx = -speed * elapsed;
      break;
  }

  x_ += dx;
  y_ += dy;

  if (!dungeon.box_walkable(collision_box())) {
    if (state_ == State::Charging) {
      state_ = State::Hold;
      timer_ = 0;
    } else if (state_ == State::Retreating) {
      state_ = State::Waiting;
      timer_ = 0;
    }

    x_ -= dx;
    y_ -= dy;
  }
}

Rect SpikeTrap::collision_box() const {
  return { x_ - kHalfTile, y_ - kHalfTile, x_ + kHalfTile, y_ + kHalfTile };
}

Rect SpikeTrap::attack_box() const {
  return { x_ - kHalfTile, y_ - kHalfTile, x_ + kHalfTile, y_ + kHalfTile };
}

int SpikeTrap::sprite_number() const {
  return 0;
}
