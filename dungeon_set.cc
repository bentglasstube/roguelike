#include "dungeon_set.h"

DungeonSet::DungeonSet() : DungeonSet(DungeonSet::random_seed()) {}

DungeonSet::DungeonSet(unsigned int seed) : floors_(), rand_(seed), current_floor_(0) {
  floors_.emplace_back(59, 79, Dungeon::TuningParams{1.0, 0.75, 0.02});
  floors_[0].generate(rand_());
}

const Dungeon& DungeonSet::get_floor(size_t floor) const {
  return floors_[floor];
}

Dungeon& DungeonSet::get_floor(size_t floor) {
  return floors_[floor];
}

const Dungeon& DungeonSet::current() const {
  return get_floor(current_floor_);
}

Dungeon& DungeonSet::current() {
  return get_floor(current_floor_);
}

size_t DungeonSet::floor() const {
  return current_floor_;
}

void DungeonSet::up() {
  if (current_floor_ > 0) --current_floor_;
}

void DungeonSet::down() {
  ++current_floor_;
  while (current_floor_ >= floors_.size()) {
    floors_.emplace_back(159, 79, Dungeon::TuningParams{1.0, 0.75, 0.02});
    floors_[current_floor_].generate(rand_());
  }
}

size_t DungeonSet::random_seed() {
  // very random
  return 2;

  // TODO replace with real thing later
  /* std::random_device r; */
  /* return r(); */
}
