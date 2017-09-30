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

Entity::Entity(std::string sprites, int cols, double x, double y) :
  sprites_(sprites, cols, kTileSize, kTileSize),
  x_(x), y_(y),
  facing_(Direction::South),
  timer_(0) {}

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
}

void Entity::draw(Graphics& graphics, int xo, int yo) const {
  const int x = x_ - kHalfTile - xo;
  const int y = y_ - kHalfTile - yo;
  sprites_.draw_ex(graphics, sprite_number(), x, y, facing_ == Direction::West, 0, 0, 0);
}

bool Entity::dead() const {
  return false;
}

int Entity::sprite_number() const {
  return 0;
}

Rect Entity::collision_box() const {
  return { 0, 0, 0, 0 };
}

Rect Entity::hit_box() const {
  return { 0, 0, 0, 0 };
}

Rect Entity::attack_box() const {
  return { 0, 0, 0, 0 };
}

Rect Entity::defense_box() const {
  return { 0, 0, 0, 0 };
}