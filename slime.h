#pragma once

#include "entity.h"

class Slime : public Entity {
  public:

    Slime(double x, double y);

    void ai(const Dungeon& dungeon, const Entity& player) override;
    void update(const Dungeon& dungeon, unsigned int elapsed) override;

  private:

    static constexpr double kMoveSpeed = 0.02;
    static constexpr int kHoldTime = 750;
    static constexpr int kSwitchTime = kHoldTime * 2;

    int sprite_number() const override;
};
