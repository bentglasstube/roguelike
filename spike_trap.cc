#include "spike_trap.h"

SpikeTrap::SpikeTrap(double x, double y) : Entity("enemies.png", 8, x, y, 1) {}

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
    state_transition(State::Attacking);
  } else if (p.second == s.second) {
    const bool west = p.first < s.first;
    const int start = west ? p.first : s.first;
    const int end = west ? s.first : p.first;

    for (int x = start + 1; x < end; ++x) {
      if (!dungeon.walkable(x, p.second)) return;
    }

    facing_ = west ? Direction::West : Direction::East;
    state_transition(State::Attacking);
  }
}

void SpikeTrap::update(Dungeon& dungeon, unsigned int elapsed) {
  Entity::update(dungeon, elapsed);

  if (state_ == State::Waiting) return;

  if (state_ == State::Holding) {
    if (timer_ > kHoldTime) {
      state_transition(State::Retreating);
      facing_ = Entity::reverse_direction(facing_);
    }
  }

  double speed = state_ == State::Attacking ? kChargingSpeed : kRetreatingSpeed;
  auto delta = Entity::delta_direction(facing_, speed * elapsed);

  // TODO also check for other spike traps
  if (!move_if_possible(dungeon, delta.first, delta.second)) {
    if (state_ == State::Attacking) {
      state_transition(State::Holding);
    } else if (state_ == State::Retreating) {
      state_transition(State::Waiting);
    }
  }
}

Rect SpikeTrap::hit_box() const {
  return { 0, 0, 0, 0 };
}

int SpikeTrap::sprite_number() const {
  return 0;
}
