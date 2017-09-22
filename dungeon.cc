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
  return { px / kTileSize, py / kTileSize };
}

void Dungeon::reveal() {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      set_visible(x, y, true);
    }
  }
}

void Dungeon::calculate_visibility(int x, int y) {
  std::queue<std::pair<int, int>> q;
  q.emplace(x, y);

  while (!q.empty()) {
    std::pair<int, int> n = q.front();
    q.pop();

    int w = n.first;
    int e = n.first;
    const int iy = n.second;

    while (walkable(w - 1, iy)) --w;
    while (walkable(e + 1, iy)) ++e;

    set_visible(w - 1, iy - 1, true);
    set_visible(w - 1, iy, true);
    set_visible(w - 1, iy + 1, true);

    for (int ix = w; ix <= e; ++ix) {
      set_visible(ix, iy - 1, true);
      set_visible(ix, iy, true);
      set_visible(ix, iy + 1, true);

      if (walkable(ix, iy - 1) && !get_cell(ix, iy - 1).visible) {
        q.emplace(ix, iy - 1);
      }
      if (walkable(ix, iy + 1) && !get_cell(ix, iy + 1).visible) {
        q.emplace(ix, iy + 1);
      }
    }

    set_visible(e + 1, iy - 1, true);
    set_visible(e + 1, iy, true);
    set_visible(e + 1, iy + 1, true);
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
