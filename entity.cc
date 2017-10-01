#include "entity.h"

Entity::Direction Entity::reverse_direction(Direction d) {
  switch (d) {
    case Direction::North: return Direction::South;
    case Direction::South: return Direction::North;
    case Direction::East: return Direction::West;
    case Direction::West: return Direction::East;
  }

  // warning if we don't have something here
  return Direction::North;
}

std::pair<double, double> Entity::delta_direction(Direction d, double amount) {
  switch (d) {
    case Direction::North: return { 0, -amount };
    case Direction::South: return { 0, amount };
    case Direction::West: return { -amount, 0 };
    case Direction::East: return { amount, 0 };
  }

  return {0, 0};
}

Entity::Entity(std::string sprites, int cols, double x, double y, int hp) :
  sprites_(sprites, cols, kTileSize, kTileSize),
  x_(x), y_(y),
  facing_(Direction::South),
  state_(State::Waiting),
  timer_(0), iframes_(0),
  maxhp_(hp), curhp_(maxhp_),
  dead_(false) {}

double Entity::x() const {
  return x_;
}

double Entity::y() const {
  return y_;
}

void Entity::set_position(double x, double y) {
  x_ = x;
  y_ = y;
}

void Entity::ai(const Dungeon&, const Entity&) {}

void Entity::update(const Dungeon&, unsigned int elapsed) {
  timer_ += elapsed;

  if (iframes_ > 0) {
    iframes_ -= elapsed;
    if (iframes_ < 0) iframes_ = 0;
  }
}

void Entity::draw(Graphics& graphics, int xo, int yo) const {
  if (iframes_ > 0 && (iframes_ / 32) % 2 == 0) return;

  const int x = x_ - kHalfTile - xo;
  const int y = y_ - kHalfTile - yo;
  sprites_.draw_ex(graphics, sprite_number(), x, y, facing_ == Direction::West, 0, 0, 0);

#ifndef NDEBUG
  hit_box().draw(graphics, 0xffffff80, false, xo, yo);
#endif
}

bool Entity::dead() const {
  return dead_;
}

void Entity::hit() {
  // TODO knockback

  if (iframes_ > 0) return;

  --curhp_;

  if (curhp_ == 0) {
    dead_ = true;
  } else {
    iframes_ = kIFrameTime;
  }
}

int Entity::sprite_number() const {
  return 0;
}

Rect Entity::collision_box() const {
  return {
    x_ - kHalfTile + 1, y_ - kHalfTile + 1,
    x_ + kHalfTile - 1, y_ + kHalfTile - 1
  };
}

Rect Entity::hit_box() const {
  return collision_box();
}

Rect Entity::attack_box() const {
  return collision_box();
}

Rect Entity::defense_box() const {
  return { 0, 0, 0, 0 };
}
