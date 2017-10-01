#include "powerup.h"

Powerup::Powerup(double x, double y, Type type, int cost) :
  Entity("ui.png", 5, x, y, 1),
  type_(type), cost_(cost) {}

void Powerup::apply(Player& target) {
  switch (type_) {
    case Type::Heart:
      target.heal(4);
      break;

    case Type::Fairy:
      target.heal(12);
      break;

    case Type::Coin:
      target.transact(1);
      break;
  }

  dead_ = true;
}

int Powerup::sprite_number() const {
  switch (type_) {
    case Type::Heart: return 4;
    case Type::Fairy: return 8 + (timer_ / 100) % 2;
    case Type::Coin: return 5;
  }

  return 0;
}
