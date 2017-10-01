#include "slime.h"

#include <random>

Slime::Slime(double x, double y) : Entity("enemies.png", 8, x, y, 1) {}

void Slime::ai(const Dungeon& dungeon, const Entity& player) {
  if (state_ == State::Walking && timer_ > kSwitchTime) {
    state_transition(State::Waiting);
  } else if (state_ == State::Waiting && timer_ > kHoldTime) {
    std::uniform_int_distribution<int> r(0, 3);
    std::random_device rd;
    facing_ = static_cast<Entity::Direction>(r(rd));
    state_transition(State::Walking);
  }
}

void Slime::update(const Dungeon& dungeon, unsigned int elapsed) {
  Entity::update(dungeon, elapsed);

  if (state_ == State::Walking) {
    auto delta = Entity::delta_direction(facing_, kMoveSpeed * elapsed);
    x_ += delta.first;
    y_ += delta.second;

    if (!dungeon.box_walkable(collision_box())) {
      x_ -= delta.first;
      y_ -= delta.second;
      state_ = State::Waiting;
      timer_ = 0;
    }
  }
}


int Slime::sprite_number() const {
  return state_ == State::Walking ? (timer_ / 250) % 3 + 1 : 1;
}
