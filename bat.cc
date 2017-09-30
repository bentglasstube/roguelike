#include "bat.h"

#include <cmath>

Bat::Bat(double x, double y) :
  Entity("enemies.png", 8, x, y, 1),
  state_(State::Waiting),
  cx_(0), cy_(0), clockwise_(true) {}

void Bat::ai(const Dungeon&, const Entity& player) {
  if (state_ == State::Resting) return;

  const double dx = x_ - player.x();
  const double dy = y_ - player.y();
  const double r = std::hypot(dx, dy);

  if (r < kAttackRadius && state_ == State::Waiting) {
    timer_ = 0;
    state_ = State::Flying;

    std::random_device rd;
    std::uniform_int_distribution<int> r(0, 1);
    clockwise_ = r(rd) == 0;

    cx_ = player.x();
    cy_ = player.y();
  } else if (r < kFollowRadius && state_ == State::Flying) {
    cx_ = player.x();
    cy_ = player.y();
  }
}

void Bat::update(const Dungeon& dungeon, unsigned int elapsed) {
  Entity::update(dungeon, elapsed);

  if (state_ == State::Flying) {
    double angle = std::atan2(y_ - cy_, x_ - cx_);
    double radius = std::hypot(y_ - cy_, x_ - cx_);
    angle += elapsed * kFlyingSpeed * (clockwise_ ? 1 : -1);
    x_ = std::cos(angle) * radius + cx_;
    y_ = std::sin(angle) * radius + cy_;

    if (timer_ > kFlyTime) {
      state_ = State::Resting;
      timer_ = 0;
    }

  } else if (state_ == State::Resting && timer_ > kRestTime) {
    state_ = State::Waiting;
    timer_ = 0;
  }
}

void Bat::draw(Graphics& graphics, int xo, int yo) const {
  Entity::draw(graphics, xo, yo);
#ifndef NDEBUG
  if (cx_ != 0 || cy_ != 0) {
    graphics.draw_line(x_ - xo, y_ - yo, cx_ - xo, cy_ - yo, 0x0000ffff);
  }
#endif
}

int Bat::sprite_number() const {
  return state_ == State::Flying ? 4 + (timer_ / 100) % 2 : 6;
}
