#include "dungeon_set.h"

#include "util.h"

#include "log.h"

DungeonSet::DungeonSet() : DungeonSet(Util::random_seed()) {}

DungeonSet::DungeonSet(unsigned int seed) : floors_(), rand_(seed), current_floor_(0) {
  DEBUG_LOG << "Dungeon set seed " << seed << "\n";
  generate_floor();
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
  if (current_floor_ >= floors_.size()) generate_floor();
}

void DungeonSet::generate_floor() {
  const int floor = floors_.size();
  DEBUG_LOG << "Generating floor " << floor << "\n";
  // TODO change parameters for each floor
  floors_.emplace_back(59, 79, Dungeon::TuningParams{1.0, 0.75,  0.02, 3});
  floors_[floor].generate(rand_());
}
