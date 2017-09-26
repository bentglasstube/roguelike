#include "dungeon.h"

#include <queue>
#include <stack>
#include <unordered_set>

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

void Dungeon::generate() {
  std::random_device r;
  generate(r());
}

void Dungeon::generate(unsigned int seed) {
  rand_.seed(seed);

  // place rooms
  const int min_room_count = params_.room_density * width_ * height_ / 2;
  int rooms = 0;
  int region = 0;
  while (rooms < min_room_count) {
    rooms += place_room(++region);
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
  // TODO why do I need -1 here?
  return { px / kTileSize, (py - 1) / kTileSize };
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

    for (int r = 1; r < 9; ++r) {
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

void Dungeon::draw(Graphics& graphics, int xo, int yo) const {
  for (int y = 0; y < height_; ++y) {
    const int gy = kTileSize * y - yo;
    if (gy < -kTileSize) continue;
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
