#pragma once

#include "graphics.h"
#include "spritemap.h"
#include "text.h"

#include "entity.h"
#include "rect.h"

class Player : public Entity {
  public:

    Player(int x, int y);

    void move(Direction direction);
    void stop();
    bool interact(Dungeon& dungeon);
    void attack();

    void transact(int amount);
    void add_key();

    void hit(Entity& source) override;
    void update(Dungeon& dungeon, unsigned int elapsed) override;
    void draw(Graphics& graphics, int xo, int yo) const override;
    void draw_hud(Graphics& graphics, int xo, int yo) const;

    Rect collision_box() const override;
    Rect hit_box() const override;
    Rect attack_box() const override; // TODO move to weapon

  private:

    static constexpr double kSpeed = 0.1;
    static constexpr int kAttackTime = 250;

    SpriteMap weapons_, ui_;
    Text text_;
    int gold_, keys_;

    int sprite_number() const override;

    void draw_weapon(Graphics& graphics, int xo, int yo) const;
};
