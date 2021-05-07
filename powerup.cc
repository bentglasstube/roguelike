#include "powerup.h"

Powerup::Powerup(double x, double y, Type type) :
  Entity("ui.png", 3, x, y, 1), type_(type) {}

void Powerup::hit(Entity& source) {
  auto player = dynamic_cast<Player*>(&source);
  if (player) apply(*player);
}

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

    case Type::Key:
      target.add_key();
      break;
  }

  dead_ = true;
}

int Powerup::sprite_number() const {
  switch (type_) {
    case Type::Heart: return 4;
    case Type::Fairy: return 7 + (timer_ / 100) % 2;
    case Type::Coin: return 5;
    case Type::Key: return 9;
  }

  return 0;
}
