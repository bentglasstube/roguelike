#include "dungeon.h"

#include <algorithm>
#include <queue>
#include <stack>
#include <unordered_set>

#include "bat.h"
#include "entity.h"
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
  rand_.seed(seed);

  // place rooms
  const int min_room_count = params_.room_density * width_ * height_ / 2;
  int rooms = 0;
  int region = 1;
  while (rooms < min_room_count) {
    const int size = place_room(region);
    if (size > 0) {
      rooms += size;
      ++region;
    }
  }

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
        int n = r(rand_) * dirs.size();
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

  // connect regions
  while (true) {
    std::vector<Connector> connectors;
    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < width_; ++x) {
        int other = is_connector(x, y);
        if (other > 0) connectors.emplace_back(Connector{x, y, other});
      }
    }

    if (connectors.empty()) break;

    const int i = r(rand_) * connectors.size();
    const Connector door = connectors[i];

    replace_region(door.region, 1);
    set_tile(door.x, door.y, Tile::DoorClosed);

    for (const auto& c : connectors) {
      if (c.region != door.region) continue;
      if (adjacent_count(c.x, c.y, Tile::DoorClosed) > 0) continue;
      if (r(rand_) >= params_.extra_doors) continue;

      set_tile(c.x, c.y, Tile::DoorClosed);
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

std::pair<int, int> Dungeon::grid_coords(int px, int py) const {
  return { px / kTileSize, py / kTileSize };
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
  calculate_visibility_roguelike(x, y);
}

void Dungeon::calculate_visibility_floodfill(int x, int y) {
  std::queue<std::pair<int, int>> q;
  q.emplace(x, y);

  while (!q.empty()) {
    auto n = q.front();
    q.pop();

    int w = n.first;
    int e = n.first;
    const int iy = n.second;
    if (iy < y - kMaxVisibility || iy > y + kMaxVisibility) continue;

    while (transparent(w - 1, iy)) --w;
    while (transparent(e + 1, iy)) ++e;

    if (w - 1 >= x - kMaxVisibility) {
      set_visible(w - 1, iy - 1, true);
      set_visible(w - 1, iy + 0, true);
      set_visible(w - 1, iy + 1, true);
    }

    for (int ix = w; ix <= e; ++ix) {
      if (ix < x - kMaxVisibility || ix > x + kMaxVisibility) continue;

      if (transparent(ix, iy - 1) && !get_cell(ix, iy - 1).visible) q.emplace(ix, iy - 1);
      if (transparent(ix, iy + 1) && !get_cell(ix, iy + 1).visible) q.emplace(ix, iy + 1);

      set_visible(ix, iy - 1, true);
      set_visible(ix, iy + 0, true);
      set_visible(ix, iy + 1, true);
    }

    if (e + 1 <= x + kMaxVisibility) {
      set_visible(e + 1, iy - 1, true);
      set_visible(e + 1, iy + 0, true);
      set_visible(e + 1, iy + 1, true);
    }
  }
}

void Dungeon::calculate_visibility_roguelike(int x, int y) {
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
        if (visible && !walkable(cx, cy)) line.add(s);
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

    if (entity->collision_box().intersect(player_hit)) {
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
  for (int y = source.top; y < source.bottom; ++y) {
    const int py = dest.top + y - source.top;
    for (int x = source.left; x < source.right; ++x) {
      const int px = dest.left + x - source.left;
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
    case Dungeon::Tile::StairsUp:
    case Dungeon::Tile::StairsDown:
      return true;
    default:
      return false;
  }
}

bool Dungeon::interact(int x, int y) {
  switch (get_cell(x, y).tile) {
    case Tile::DoorClosed:
      set_tile(x, y, Tile::DoorOpen);
      return true;

    case Tile::DoorOpen:
      set_tile(x, y, Tile::DoorClosed);
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

Dungeon::Cell Dungeon::get_cell(int x, int y) const {
  if (x < 0 || x >= width_) return { Dungeon::Tile::OutOfBounds, 0, false, false };
  if (y < 0 || y >= height_) return { Dungeon::Tile::OutOfBounds, 0, false, false };
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
        const int x = rx(rand_) * kTileSize + kHalfTile;
        const int y = ry(rand_) * kTileSize + kHalfTile;
        entities_.emplace_back(new Slime(x, y));
      }
    }

    if (rand_percent(rand_) < 50) {
      // bat room

      const int bats = rcount(rand_);
      for (int i = 0; i < bats; ++i) {
        const int x = rx(rand_) * kTileSize + kHalfTile;
        const int y = ry(rand_) * kTileSize + kHalfTile;
        entities_.emplace_back(new Bat(x, y));
      }
    }
  }

  return w * h;
}

int Dungeon::is_connector(int x, int y) const {
  if (get_cell(x, y).tile != Tile::Wall) return 0;

  std::unordered_set<int> near;
  near.insert(get_cell(x - 1, y).region);
  near.insert(get_cell(x + 1, y).region);
  near.insert(get_cell(x, y - 1).region);
  near.insert(get_cell(x, y + 1).region);

  near.erase(0);
  if (near.size() == 2 && near.count(1) == 1) {
    near.erase(1);
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
  const int x1 = r.left / kTileSize;
  const int x2 = r.right / kTileSize;
  const int y1 = r.top / kTileSize;
  const int y2 = r.bottom / kTileSize;

  return walkable(x1, y1) && walkable(x1, y2) &&
         walkable(x2, y1) && walkable(x2, y2);
}

bool Dungeon::box_visible(const Rect& r) const {
  const int x1 = r.left / kTileSize;
  const int x2 = r.right / kTileSize;
  const int y1 = r.top / kTileSize;
  const int y2 = r.bottom / kTileSize;

  return get_cell(x1, y1).visible ||
         get_cell(x1, y2).visible ||
         get_cell(x2, y1).visible ||
         get_cell(x2, y2).visible;
}

int Dungeon::get_cell_color(int x, int y) const {
  const auto cell = get_cell(x, y);
  if (!cell.seen) return 0x000000ff;

  switch (cell.tile) {
    case Tile::Wall:
      return 0xaaaaaaff;
    case Tile::DoorClosed:
      return 0x552200ff;
    case Tile::StairsUp:
    case Tile::StairsDown:
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
