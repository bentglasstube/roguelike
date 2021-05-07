#pragma once

#include "entity.h"

class Bat : public Entity {
  public:

    Bat(double x, double y);

    void ai(const Dungeon& dungeon, const Entity& player) override;
    void update(Dungeon& dungeon, unsigned int elapsed) override;
    void draw(Graphics& graphics, int xo, int yo) const override;

  private:

    static constexpr double kFlyingSpeed = 0.002;
    static constexpr double kAttackRadius = 50;
    static constexpr double kFollowRadius = kAttackRadius * 1.5;
    static constexpr int kRestTime = 1500;
    static constexpr int kFlyTime = kRestTime * 3;

    double cx_, cy_;
    bool clockwise_;

    int sprite_number() const override;
    bool collision(const Dungeon&) const override;
};

