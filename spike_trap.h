#pragma once

#include "entity.h"

class SpikeTrap : public Entity {
  public:

    SpikeTrap(double x, double y);

    void ai(const Dungeon& dungeon, const Entity& player) override;
    void update(Dungeon& dungeon, unsigned int elapsed) override;

    Rect hit_box() const;

  private:

    static constexpr double kChargingSpeed = 0.15;
    static constexpr double kRetreatingSpeed = kChargingSpeed / 2;
    static constexpr int kHoldTime = 500;

    int sprite_number() const override;
};
