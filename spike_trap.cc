#include "spike_trap.h"

SpikeTrap::SpikeTrap(double x, double y) :
  Entity("enemies.png", 8, x, y, 1),
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
  } else if (p.second == s.second) {
    const bool west = p.first < s.first;
    const int start = west ? p.first : s.first;
    const int end = west ? s.first : p.first;

    for (int x = start + 1; x < end; ++x) {
      if (!dungeon.walkable(x, p.second)) return;
    }

    facing_ = west ? Direction::West : Direction::East;
    state_ = State::Charging;
  }
}

void SpikeTrap::update(const Dungeon& dungeon, unsigned int elapsed) {
  Entity::update(dungeon, elapsed);

  if (state_ == State::Waiting) return;

  if (state_ == State::Hold) {
    if (timer_ > kHoldTime) {
      state_ = State::Retreating;
      timer_ = 0;
      facing_ = Entity::reverse_direction(facing_);
    }
  }

  double speed = state_ == State::Charging ? kChargingSpeed : kRetreatingSpeed;
  auto delta = Entity::delta_direction(facing_, speed * elapsed);

  x_ += delta.first;
  y_ += delta.second;

  // TODO also check for other spike traps
  if (!dungeon.box_walkable(collision_box())) {
    if (state_ == State::Charging) {
      state_ = State::Hold;
      timer_ = 0;
    } else if (state_ == State::Retreating) {
      state_ = State::Waiting;
      timer_ = 0;
    }

    x_ -= delta.first;
    y_ -= delta.second;
  }
}

Rect SpikeTrap::hit_box() const {
  return { 0, 0, 0, 0 };
}

int SpikeTrap::sprite_number() const {
  return 0;
}
