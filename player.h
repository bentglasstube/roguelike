#pragma once

#include "graphics.h"
#include "spritemap.h"

#include "entity.h"

class Player : public Entity {
  public:

    Player(int x, int y);

    void move(Direction direction);
    void stop();
    bool interact(Dungeon& dungeon);
    void attack();

    void update(const Dungeon& dungeon, unsigned int elapsed) override;
    void draw(Graphics& graphics, int xo, int yo) const override;

  private:

    enum class State { Standing, Walking, Attacking };

    static constexpr double kSpeed = 0.1;
    static constexpr int kAttackTime = 250;

    SpriteMap weapons_;
    State state_;

    int sprite_number() const override;

    Rect collision_box() const override;
    Rect hit_box() const override;
    Rect attack_box() const override; // TODO move to weapon

    void draw_weapon(Graphics& graphics, int xo, int yo) const;
};
