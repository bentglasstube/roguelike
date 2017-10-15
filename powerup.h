#pragma once

#include "entity.h"
#include "player.h"

class Powerup : public Entity {
  public:

    enum class Type { Heart, Fairy, Coin, Key };

    Powerup(double x, double y, Type type, int cost);

    void hit(Entity& source);
    void apply(Player& target);

  private:

    Type type_;
    int cost_;

    int sprite_number() const override;

};
