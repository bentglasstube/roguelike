#include "dungeon.h"

#include <algorithm>
#include <stack>
#include <unordered_set>

#include "bat.h"
#include "entity.h"
#include "log.h"
#include "powerup.h"
#include "slime.h"
#include "spike_trap.h"

Dungeon::Dungeon(int width, int height, TuningParams params) :
  width_(width), height_(height), params_(params),
  tiles_("tiles.png", 4, kTileSize, kTileSize)
{
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      cells_[y][x] = { Dungeon::Tile::Wall, 0, false, false };
    }
  }
}

void Dungeon::generate(unsigned int seed) {
  DEBUG_LOG << "Generating dungeon with seed " << seed << "\n";
  rand_.seed(seed);

  // place rooms
  const int min_room_count = (int)(params_.room_density * width_ * height_ / 2);
  int rooms = 0;
  int region = 1;
  while (rooms < min_room_count) {
    const int size = place_room(region);
    if (size > 0) {
      rooms += size;
      ++region;
    }
  }
  DEBUG_LOG << "Placed " << region << "rooms\n";

  // generate hallways
  std::stack<Position> stack;
  std::uniform_real_distribution<double> r(0, 1);
  Direction last_dir = Direction::North;
  Position pos = find_open_space();
  ++region;

  while (pos.x > 0) {
    set_tile(pos.x, pos.y, Tile::Hallway);
    set_region(pos.x, pos.y, region);
    // TODO place enemies in hallways occasionally

    std::unordered_set<Direction> dirs;
    if (get_cell(pos.x, pos.y - 2).tile == Tile::Wall)
      dirs.insert(Direction::North);
    if (get_cell(pos.x, pos.y + 2).tile == Tile::Wall)
      dirs.insert(Direction::South);
    if (get_cell(pos.x - 2, pos.y).tile == Tile::Wall)
      dirs.insert(Direction::East);
    if (get_cell(pos.x + 2, pos.y).tile == Tile::Wall)
      dirs.insert(Direction::West);

    if (dirs.size() > 1) stack.push(pos);

    if (dirs.size() > 0) {
      Direction dir;
      if (dirs.count(last_dir) == 1 && r(rand_) < params_.straightness) {
        dir = last_dir;
      } else {
        int n = (int)(r(rand_) * dirs.size());
        auto it = dirs.begin();
        while (n > 0) {
          std::advance(it, 1);
          --n;
        }
        dir = *(it);
      }

      last_dir = dir;

      switch (dir) {
        case Direction::North:
          set_tile(pos.x, pos.y - 1, Tile::Hallway);
          set_region(pos.x, pos.y - 1, region);
          pos.y -= 2;
          break;

        case Direction::South:
          set_tile(pos.x, pos.y + 1, Tile::Hallway);
          set_region(pos.x, pos.y + 1, region);
          pos.y += 2;
          break;

        case Direction::East:
          set_tile(pos.x - 1, pos.y, Tile::Hallway);
          set_region(pos.x - 1, pos.y, region);
          pos.x -= 2;
          break;

        case Direction::West:
          set_tile(pos.x + 1, pos.y, Tile::Hallway);
          set_region(pos.x + 1, pos.y, region);
          pos.x += 2;
          break;
      }
    } else {
      if (stack.empty()) {
        pos = find_open_space();
        ++region;
      } else {
        pos = stack.top();
        stack.pop();
      }
    }
  }
  DEBUG_LOG << "Hallways added, " << region << " regions\n";

  // connect regions
  DEBUG_LOG << "Connecting regions within " << params_.sections << " sections\n";
  bool placed = true;
  while (placed) {
    placed = false;

    for (int i = 1; i <= params_.sections; ++i) {
      auto connectors = get_connectors(i, params_.sections);
      if (connectors.empty()) {
        DEBUG_LOG << "No connections to region " << i << "\n";
        continue;
      }

      placed = true;
      const int j = (int)(r(rand_) * connectors.size());
      const Connector door = connectors[j];

      replace_region(door.region, i);
      set_tile(door.x, door.y, Tile::DoorClosed);

      DEBUG_LOG << "Connected region " << door.region << " to " << i << "\n";

      for (const auto& c : connectors) {
        if (c.region != door.region) continue;
        if (adjacent_count(c.x, c.y, Tile::DoorClosed) > 0) continue;
        if (r(rand_) >= params_.extra_doors) continue;

        set_tile(c.x, c.y, Tile::DoorClosed);
      }
    }
  }

  // place locks and keys
  DEBUG_LOG << "Placing locks and keys\n";
  while (true) {
    auto connectors = get_connectors(1, 0);
    if (connectors.empty()) break;

    std::map<int, std::vector<Connector>> doors;
    std::unordered_set<int> regions_found;
    for (const auto& c : connectors) {
      doors[c.region].push_back(c);
      regions_found.insert(c.region);
    }

    if (regions_found.empty()) {
      DEBUG_LOG << "No more sections to connect\n";
      break;
    }

    DEBUG_LOG << "Found connectors to sections ";

    for (int i : regions_found) {
      DEBUG_LOG << i << " ";
      const auto door = doors[i][(int)(r(rand_) * doors[i].size())];
      set_tile(door.x, door.y, Tile::DoorLocked);
      place_key();
    }
    DEBUG_LOG << "\n";

    for (int i : regions_found) {
      replace_region(i, 1);
      DEBUG_LOG << "Connected section " << i << "\n";
    }
  }

  // clean up dead ends
  bool removed = true;
  while (removed) {
    removed = false;
    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < width_; ++x) {
        if (is_dead_end(x, y)) {
          removed = true;
          set_tile(x, y, Tile::Wall);
        }
      }
    }
  }
}

Dungeon::Position Dungeon::grid_coords(double px, double py) const {
  return { (int)(px / kTileSize), (int)(py / kTileSize) };
}

void Dungeon::reveal() {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      set_visible(x, y, true);
    }
  }
}

void Dungeon::hide() {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      set_visible(x, y, false);
    }
  }
}

// calculates x and y offsets for each octant
std::pair<int, int> transform(int r, int c, int octant) {
  switch (octant) {
    case 0: return { +c, -r };
    case 1: return { +r, -c };
    case 2: return { +r, +c };
    case 3: return { +c, +r };
    case 4: return { -c, +r };
    case 5: return { -r, +c };
    case 6: return { -r, -c };
    case 7: return { -c, -r };
  }

  return { 0, 0 };
}

bool Dungeon::Shadow::contains(const Shadow& other) const {
  return start <= other.start && end >= other.end;
}

Dungeon::ShadowLine::ShadowLine() : shadows_() {}

bool Dungeon::ShadowLine::is_shadowed(const Shadow& shadow) const {
  for (const auto& s : shadows_) {
    if (s.contains(shadow)) return true;
  }
  return false;
}

void Dungeon::ShadowLine::add(const Shadow& shadow) {
  size_t i = 0;
  for (i = 0; i < shadows_.size(); ++i) {
    if (shadows_[i].start >= shadow.start) break;
  }

  auto* prev = (i > 0 && shadows_[i - 1].end > shadow.start) ? &shadows_[i - 1] : nullptr;
  auto* next = (i < shadows_.size() && shadows_[i].start < shadow.end) ? &shadows_[i] : nullptr;

  if (next) {
    if (prev) {
      prev->end = next->end;
      shadows_.erase(shadows_.begin() + i);
    } else {
      next->start = shadow.start;
    }
  } else {
    if (prev) {
      prev->end = shadow.end;
    } else {
      shadows_.insert(shadows_.begin() + i, shadow);
    }
  }
}

void Dungeon::calculate_visibility(int x, int y) {
  hide();
  set_visible(x, y, true);
  for (int octant = 0; octant < 8; ++octant) {
    ShadowLine line;

    for (int r = 1; r < kMaxVisibility; ++r) {
      for (int c = 0; c <= r; ++c) {
        Shadow s = { c / (double)(r + 1), (c + 1) / (double)r};

        const auto offset = transform(r, c, octant);
        const int cx = x + offset.first;
        const int cy = y + offset.second;

        const bool visible = !line.is_shadowed(s);
        set_visible(cx, cy, visible);
        if (visible && !transparent(cx, cy)) line.add(s);
      }
    }
  }
}

Dungeon::Position Dungeon::find_tile(Tile tile) const {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      if (cells_[y][x].tile == tile) return {x, y};
    }
  }

  return {-1, -1};
}

bool Dungeon::any_entity_at(int x, int y) const {
  return any_entity_at(x, y, [](const std::unique_ptr<Entity>& e){ return true; });
}

bool Dungeon::any_entity_at(int x, int y, std::function<bool(const std::unique_ptr<Entity>&)> pred) const {
  return std::any_of(entities_.cbegin(), entities_.cend(),
      [this, x, y, pred](const std::unique_ptr<Entity>& e) {
        const auto p = grid_coords(e->x(), e->y());
        return p.x == x && p.y == y && pred(e);
      });
}

void Dungeon::update(Entity& player, unsigned int elapsed) {
  const Rect player_attack = player.attack_box();
  const Rect player_hit = player.hit_box();

  for (auto& entity : entities_) {
    entity->ai(*this, player);
    entity->update(*this, elapsed);

    if (!player_attack.empty()) {
      if (entity->hit_box().intersect(player_attack)) {
        entity->hit(player);
      }
    }

    if (entity->alive() && entity->collision_box().intersect(player_hit)) {
      player.hit(*entity);
    }
  }

  entities_.erase(std::remove_if( entities_.begin(), entities_.end(),
        [](const std::unique_ptr<Entity>& e){return e->dead();}), entities_.end());
}

void Dungeon::draw(Graphics& graphics, int hud_height, int xo, int yo) const {
  for (int y = 0; y < height_; ++y) {
    const int gy = kTileSize * y - yo;
    if (gy < hud_height - kTileSize) continue;
    if (gy > graphics.height()) break;

    for (int x = 0; x < width_; ++x) {
      const int gx = kTileSize * x - xo;
      if (gx < -kTileSize) continue;
      if (gx > graphics.width()) break;

      if (cells_[y][x].seen) {
        tiles_.draw(graphics, static_cast<int>(cells_[y][x].tile), gx, gy);
        if (!cells_[y][x].visible) {
          SDL_Rect r = { gx, gy, kTileSize, kTileSize };
          graphics.draw_rect(&r, 0x00000080, true);
        }
      }
    }
  }

  for (const auto& entity : entities_) {
    if (box_visible(entity->collision_box())) {
      entity->draw(graphics, xo, yo);
    }
  }
}

void Dungeon::draw_map(Graphics& graphics, const Rect& source, const Rect& dest) const {
  for (int y = (int)source.top; y < (int)source.bottom; ++y) {
    const int py = (int)dest.top + y - (int)source.top;
    for (int x = (int)source.left; x < (int)source.right; ++x) {
      const int px = (int)dest.left + x - (int)source.left;
      graphics.draw_pixel(px, py, get_cell_color(x, y));
    }
  }

  dest.draw(graphics, 0xffffffff, false, 0, 0);
}

bool Dungeon::walkable(int x, int y) const {
  switch (get_cell(x, y).tile) {
    case Dungeon::Tile::Room:
    case Dungeon::Tile::Hallway:
    case Dungeon::Tile::DoorOpen:
    case Dungeon::Tile::StairsUp:
    case Dungeon::Tile::StairsDown:
      return true;
    default:
      return false;
  }
}

bool Dungeon::transparent(int x, int y) const {
  switch (get_cell(x, y).tile) {
    case Dungeon::Tile::Room:
    case Dungeon::Tile::Hallway:
    case Dungeon::Tile::DoorOpen:
    case Dungeon::Tile::StairsUp:
    case Dungeon::Tile::StairsDown:
    case Dungeon::Tile::ChestOpen:
    case Dungeon::Tile::ChestClosed:
      return true;
    default:
      return false;
  }
}

void Dungeon::set_tile(int x, int y, Dungeon::Tile tile) {
  if (x < 0 || x >= width_) return;
  if (y < 0 || y >= height_) return;
  cells_[y][x].tile = tile;
}

void Dungeon::set_region(int x, int y, int region) {
  if (x < 0 || x >= width_) return;
  if (y < 0 || y >= height_) return;
  cells_[y][x].region = region;
}

void Dungeon::set_visible(int x, int y, bool visible) {
  if (x < 0 || x >= width_) return;
  if (y < 0 || y >= height_) return;
  cells_[y][x].visible = visible;
  if (visible) cells_[y][x].seen = true;
}

const Dungeon::Cell& Dungeon::get_cell(int x, int y) const {
  if (x < 0 || x >= width_) return kBadCell;
  if (y < 0 || y >= height_) return kBadCell;
  return cells_[y][x];
}

Dungeon::Position Dungeon::find_open_space() const {
  for (int y = 1; y < height_; y += 2) {
    for (int x = 1; x < width_; x += 2) {
      if (cells_[y][x].tile == Dungeon::Tile::Wall) {
        return {x, y};
      }
    }
  }
  return {-1, -1};
}

int Dungeon::random_odd(int min, int max) {
  std::uniform_int_distribution<int> r(min / 2, max / 2);
  return r(rand_) * 2 + 1;
}

int Dungeon::place_room(int region) {
  int x = random_odd(1, width_);
  int y = random_odd(1, height_);

  const int size = random_odd(7, 17);
  int w = size;
  int h = size;

  if (random_odd(1, 3) == 1) {
    w += random_odd(3, 5) + 1;
  } else {
    h += random_odd(3, 5) + 1;
  }

  if (x + w >= width_) x -= w - 1;
  if (y + h >= height_) y -= h - 1;

  for (int iy = 0; iy < h; ++iy) {
    for (int ix = 0; ix < w; ++ix) {
      if (get_cell(x + ix, y + iy).tile != Tile::Wall) return 0;
    }
  }

  for (int iy = 0; iy < h; ++iy) {
    for (int ix = 0; ix < w; ++ix) {
      set_tile(x + ix, y + iy, Tile::Room);
      set_region(x + ix, y + iy, region);
    }
  }

  std::uniform_int_distribution<int> rx(x + 1, x + w - 1);
  std::uniform_int_distribution<int> ry(y + 1, y + h - 1);

  if (region == 1) {
    set_tile(rx(rand_), ry(rand_), Tile::StairsUp);
  } else if (region == 2) {
    set_tile(rx(rand_), ry(rand_), Tile::StairsDown);
  } else if (region <= params_.sections) {
    set_tile(rx(rand_), ry(rand_), Tile::ChestClosed);
    // TODO add items to chests
  } else {
    // TODO implement more room types
    //
    // Ideas:
    //
    // treasure chest in room
    // obstacles? blocks/water
    // Enemy sets
    //   Spike traps in room
    //   might depend on the room shape that is decided

    std::uniform_int_distribution<int> rand_percent(0, 99);
    std::uniform_int_distribution<int> rcount(2, 8);

    if (rand_percent(rand_) < 25) {
      // spike trap room
      const int x1 = x * kTileSize + kHalfTile;
      const int x2 = (x + w) * kTileSize - kHalfTile;
      const int y1 = y * kTileSize + kHalfTile;
      const int y2 = (y + h) * kTileSize - kHalfTile;

      entities_.emplace_back(new SpikeTrap(x1, y1));
      entities_.emplace_back(new SpikeTrap(x1, y2));
      entities_.emplace_back(new SpikeTrap(x2, y1));
      entities_.emplace_back(new SpikeTrap(x2, y2));
    }

    if (rand_percent(rand_) < 50) {
      // slime room

      const int slimes = rcount(rand_);
      for (int i = 0; i < slimes; ++i) {
        const int ex = rx(rand_) * kTileSize + kHalfTile;
        const int ey = ry(rand_) * kTileSize + kHalfTile;
        entities_.emplace_back(new Slime(ex, ey));
      }
    }

    if (rand_percent(rand_) < 50) {
      // bat room

      const int bats = rcount(rand_);
      for (int i = 0; i < bats; ++i) {
        const int ex = rx(rand_) * kTileSize + kHalfTile;
        const int ey = ry(rand_) * kTileSize + kHalfTile;
        entities_.emplace_back(new Bat(ex, ey));
      }
    }
  }

  return w * h;
}

int Dungeon::is_connector(int x, int y, int region) const {
  if (get_cell(x, y).tile != Tile::Wall) return 0;

  std::unordered_set<int> near;
  near.insert(get_cell(x - 1, y).region);
  near.insert(get_cell(x + 1, y).region);
  near.insert(get_cell(x, y - 1).region);
  near.insert(get_cell(x, y + 1).region);

  near.erase(0);
  if (near.size() == 2 && near.count(region) == 1) {
    near.erase(region);
    return *(near.begin());
  }

  return 0;
}

void Dungeon::replace_region(int from, int to) {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      if (get_cell(x, y).region == from) set_region(x, y, to);
    }
  }
}

void Dungeon::place_key() {
  std::vector<Position> places;
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      const auto& cell = get_cell(x, y);
      if (cell.region == 1 && cell.tile == Tile::Room) {
        places.push_back({x, y});
      }
    }
  }

  // TODO handle this condition better
  if (places.empty()) {
    DEBUG_LOG << "Unable to place key\n";
    return;
  }

  std::uniform_int_distribution<int> r(0, places.size() - 1);
  const Position& p = places[r(rand_)];
  const int kx = p.x * kTileSize + kHalfTile;
  const int ky = p.y * kTileSize + kHalfTile;
  entities_.emplace_back(new Powerup(kx, ky, Powerup::Type::Key, 0));
}

int Dungeon::adjacent_count(int x, int y, Tile tile) const {
  int count = 0;
  if (get_cell(x - 1, y).tile == tile) ++count;
  if (get_cell(x + 1, y).tile == tile) ++count;
  if (get_cell(x, y - 1).tile == tile) ++count;
  if (get_cell(x, y + 1).tile == tile) ++count;

  return count;
}

bool Dungeon::is_dead_end(int x, int y) const {
  return get_cell(x, y).tile != Tile::Wall &&
    adjacent_count(x, y, Tile::Wall) >= 3;
}

bool Dungeon::box_walkable(const Rect& r) const {
  const auto a = grid_coords(r.left, r.top);
  const auto b = grid_coords(r.right, r.bottom);

  return walkable(a.x, a.y) && walkable(a.x, b.y) &&
         walkable(b.x, a.y) && walkable(b.x, b.y);
}

void Dungeon::open_door(int x, int y) {
  switch (get_cell(x, y).tile) {
    case Tile::DoorLocked:
    case Tile::DoorClosed:
      cells_[y][x].tile = Tile::DoorOpen;
      break;
    default:
      // do nothing
      break;
  }
}

void Dungeon::close_door(int x, int y) {
  switch (get_cell(x, y).tile) {
    case Tile::DoorOpen:
      cells_[y][x].tile = Tile::DoorClosed;
      break;
    default:
      // do nothing
      break;
  }
}

void Dungeon::open_chest(int x, int y) {
  switch (get_cell(x, y).tile) {
    case Tile::ChestClosed:
      cells_[y][x].tile = Tile::ChestOpen;
      // TODO give treasure to player
      break;
    default:
      // do nothing
      break;
  }
}

bool Dungeon::box_visible(const Rect& r) const {
  const auto a = grid_coords(r.left, r.top);
  const auto b = grid_coords(r.right, r.bottom);

  return get_cell(a.x, a.y).visible ||
         get_cell(a.x, b.y).visible ||
         get_cell(b.x, a.y).visible ||
         get_cell(b.x, b.y).visible;
}

int Dungeon::get_cell_color(int x, int y) const {
  const auto cell = get_cell(x, y);

  if (!cell.seen) return 0x000000ff;
  if (cell.visible && any_entity_at(x, y)) return 0xff0000ff;

  switch (cell.tile) {
    case Tile::Wall:
      return 0xaaaaaaff;
    case Tile::DoorClosed:
      return 0x552200ff;
    case Tile::DoorLocked:
      return 0xffff00ff;
    case Tile::StairsUp:
    case Tile::StairsDown:
    case Tile::ChestClosed:
      return 0xffffffff;
    default:
      return 0x885511ff;
  }
}

void Dungeon::add_drop(double x, double y) {
  std::uniform_int_distribution<int> r(0, 9);
  std::random_device rd;
  int p = r(rd);

  if (p < 2) {
    entities_.emplace_back(new Powerup(x, y, Powerup::Type::Heart, 0));
  } else if (p < 4) {
    entities_.emplace_back(new Powerup(x, y, Powerup::Type::Coin, 0));
  }
}

std::vector<Dungeon::Connector> Dungeon::get_connectors(int region, int min) const {
  std::vector<Connector> connectors;
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      int other = is_connector(x, y, region);
      if (other > min) connectors.emplace_back(Connector{x, y, other});
    }
  }
  return connectors;
}

constexpr Dungeon::Cell Dungeon::kBadCell;
