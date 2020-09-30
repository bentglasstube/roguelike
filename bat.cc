#include "bat.h"

#include <cmath>
#include <random>

Bat::Bat(double x, double y) :
  Entity("enemies.png", 8, x, y, 4),
  cx_(0), cy_(0), clockwise_(true) {}

void Bat::ai(const Dungeon&, const Entity& player) {
  if (state_ == State::Holding) return;

  const double dx = x_ - player.x();
  const double dy = y_ - player.y();
  const double rad = std::hypot(dx, dy);

  if (rad < kAttackRadius && state_ == State::Waiting) {
    state_transition(State::Attacking);

    std::uniform_int_distribution<int> r(0, 1);
    clockwise_ = r(rd_) == 0;

    cx_ = player.x();
    cy_ = player.y();
  } else if (rad < kFollowRadius && state_ == State::Attacking) {
    cx_ = player.x();
    cy_ = player.y();
  }
}

void Bat::update(Dungeon& dungeon, unsigned int elapsed) {
  Entity::update(dungeon, elapsed);

  if (state_ == State::Attacking) {
    double angle = std::atan2(y_ - cy_, x_ - cx_);
    double radius = std::hypot(y_ - cy_, x_ - cx_);
    angle += elapsed * kFlyingSpeed * (clockwise_ ? 1 : -1);
    x_ = std::cos(angle) * radius + cx_;
    y_ = std::sin(angle) * radius + cy_;

    if (timer_ > kFlyTime) state_transition(State::Holding);
  } else if (state_ == State::Holding && timer_ > kRestTime) {
    state_transition(State::Waiting);
  }
}

void Bat::draw(Graphics& graphics, int xo, int yo) const {
  Entity::draw(graphics, xo, yo);
#ifndef NDEBUG
  if (cx_ != 0 || cy_ != 0) {
    const Graphics::Point p1 = { (int)x_ - xo, (int)y_ - yo };
    const Graphics::Point p2 = { (int)cx_ - xo, (int)cy_ - yo };
    graphics.draw_line(p1, p2, 0x0000ffff);
  }
#endif
}

int Bat::sprite_number() const {
  return state_ == State::Attacking ? 4 + (timer_ / 100) % 2 : 6;
}

bool Bat::collision(const Dungeon&) const {
  return false;
}
