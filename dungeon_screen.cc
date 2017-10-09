#include "dungeon_screen.h"

DungeonScreen::DungeonScreen() :
  text_("text.png"),
  camera_(),
  dungeon_set_(),
  player_(0, 0),
  state_(State::FadeIn),
  take_stairs_(false),
  timer_(0)
{
  move_player_to_tile(Dungeon::Tile::StairsUp);
}

bool DungeonScreen::update(const Input& input, Audio&, unsigned int elapsed) {
  Dungeon& dungeon = dungeon_set_.current();
  auto pos = dungeon.grid_coords(player_.x(), player_.y());
  auto tile = dungeon.get_cell(pos.first, pos.second).tile;

  if (state_ == State::FadeIn) {
    timer_ += elapsed;
    if (timer_ > kFadeTimer) {
      state_ = State::Playing;
      timer_ = 0;
    }
  } else if (state_ == State::FadeOut) {
    timer_ += elapsed;
    if (timer_ > kFadeTimer) {
      if (tile == Dungeon::Tile::StairsUp) {
        dungeon_set_.up();
        move_player_to_tile(Dungeon::Tile::StairsDown);
      } else if (tile == Dungeon::Tile::StairsDown) {
        dungeon_set_.down();
        move_player_to_tile(Dungeon::Tile::StairsUp);
      }

      timer_ = 0;
      state_ = State::FadeIn;
    }
  } else {
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
      if (!player_.interact(dungeon)) player_.attack();
    }
  }

  player_.update(dungeon, elapsed);
  dungeon.update(player_, elapsed);
  camera_.update(player_);

  if (tile == Dungeon::Tile::StairsUp) {
    if (take_stairs_) {
      take_stairs_ = false;
      if (dungeon_set_.floor() == 0) {
        // TODO show message about not going up
        std::cerr << "I can't leave yet\n";
      } else {
        player_.stop();
        state_ = State::FadeOut;
      }
    }
  } else if (tile == Dungeon::Tile::StairsDown) {
    if (take_stairs_) {
      take_stairs_ = false;
      player_.stop();
      state_ = State::FadeOut;
    }
  } else {
    take_stairs_ = true;
  }

  dungeon.calculate_visibility(pos.first, pos.second);

  return true;
}

void DungeonScreen::draw(Graphics& graphics) const {
  const int xo = camera_.xoffset();
  const int yo = camera_.yoffset();
  const Dungeon& dungeon = dungeon_set_.current();

  dungeon.draw(graphics, kHudHeight, xo, yo);
  player_.draw(graphics, xo, yo);

  if (state_ == State::FadeIn || state_ == State::FadeOut) {
    const double pct = timer_ / (double)kFadeTimer;
    const int width = (state_ == State::FadeOut ? pct : 1 - pct) * graphics.width() / 2;

    SDL_Rect r = { 0, 0, width, graphics.height() };
    graphics.draw_rect(&r, 0x000000ff, true);
    r.x = graphics.width() - width;
    graphics.draw_rect(&r, 0x000000ff, true);
  }

  SDL_Rect r = {0, 0, graphics.width(), kHudHeight};
  graphics.draw_rect(&r, 0x000000ff, true);

  const auto p = dungeon.grid_coords(player_.x(), player_.y());
  const Rect map_region = {
    (double)(p.first - kMapWidth / 2),
    (double)(p.second - kMapHeight / 2),
    (double)(p.first + kMapWidth / 2),
    (double)(p.second + kMapHeight / 2),
  };
  dungeon.draw_map(graphics, map_region, { 0, 0, kMapWidth, kMapHeight });
  player_.draw_hud(graphics, kMapWidth, 0);
  text_.draw(graphics, "LVL " + std::to_string(1 + dungeon_set_.floor()), kMapWidth + 16, 16);
}

Screen* DungeonScreen::next_screen() const {
  return nullptr;
}

std::string DungeonScreen::get_music_track() const {
  return "";
}

void DungeonScreen::move_player_to_tile(Dungeon::Tile tile) {
  auto p = dungeon_set_.current().find_tile(tile);
  player_.set_position(p.x * 16 + 8, p.y * 16 + 8);
}
