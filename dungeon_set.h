#pragma once

#include "dungeon.h"
#include "entity.h"

class DungeonSet {
  public:

    DungeonSet();
    DungeonSet(unsigned int seed);

    const Dungeon& get_floor(size_t floor) const;
    Dungeon& get_floor(size_t floor);
    const Dungeon& current() const;
    Dungeon& current();

    size_t floor() const;

    void up();
    void down();

  private:

    static size_t random_seed();

    std::vector<Dungeon> floors_;
    std::default_random_engine rand_;
    size_t current_floor_;

    void generate_floor();
};
