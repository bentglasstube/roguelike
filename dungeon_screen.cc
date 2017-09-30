#include "dungeon_screen.h"

DungeonScreen::DungeonScreen() :
  text_("text.png"),
  camera_(),
  dungeon_(159, 79, { 1.0, 0.75, 0.02 }),
  player_(0, 0)
{
  dungeon_.generate();
  auto p = dungeon_.find_tile(Dungeon::Tile::StairsUp);
  player_.set_position(p.x * 16 + 8, p.y * 16 + 8);
}

bool DungeonScreen::update(const Input& input, Audio&, unsigned int elapsed) {
  if (input.key_held(Input::Button::Left)) {
    player_.move(Player::Direction::West);
  } else if (input.key_held(Input::Button::Right)) {
    player_.move(Player::Direction::East);
  } else if (input.key_held(Input::Button::Up)) {
    player_.move(Player::Direction::North);
  } else if (input.key_held(Input::Button::Down)) {
    player_.move(Player::Direction::South);
  } else {
    player_.stop();
  }

  if (input.key_pressed(Input::Button::A)) {
    if (!player_.interact(dungeon_)) player_.attack();
  }

  player_.update(dungeon_, elapsed);
  dungeon_.update(player_, elapsed);
  camera_.update(player_);

  auto c = dungeon_.grid_coords(player_.x(), player_.y());
  dungeon_.calculate_visibility(c.first, c.second);

  return true;
}

void DungeonScreen::draw(Graphics& graphics) const {
  const int xo = camera_.xoffset();
  const int yo = camera_.yoffset();

  dungeon_.draw(graphics, kHudHeight, xo, yo);
  player_.draw(graphics, xo, yo);

  SDL_Rect r = {0, 0, graphics.width(), kHudHeight};
  graphics.draw_rect(&r, 0x000000ff, true);

  const auto p = dungeon_.grid_coords(player_.x(), player_.y());
  const Rect map_region = {
    (double)(p.first - kMapWidth / 2),
    (double)(p.second - kMapHeight / 2),
    (double)(p.first + kMapWidth / 2),
    (double)(p.second + kMapHeight / 2),
  };
  dungeon_.draw_map(graphics, map_region, { 0, 0, kMapWidth, kMapHeight });
  player_.draw_hud(graphics, kMapWidth, 0);

  graphics.draw_rect(&r, 0xffffffff, false);
}

Screen* DungeonScreen::next_screen() const {
  return nullptr;
}

std::string DungeonScreen::get_music_track() const {
  return "";
}
