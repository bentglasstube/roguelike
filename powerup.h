#pragma once

#include "entity.h"
#include "player.h"

class Powerup : public Entity {
  public:

    enum class Type { Heart, Fairy, Coin, Key };

    Powerup(double x, double y, Type type);

    void hit(Entity& source) override;
    void apply(Player& target);

  private:

    Type type_;

    int sprite_number() const override;

};
