#include "dungeon_screen.h"

DungeonScreen::DungeonScreen() :
  text_("text.png"),
  camera_(),
  dungeon_(159, 79, { 1.0, 0.75, 0.02 }),
  player_(248, 80)
{
  dungeon_.generate();
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
  camera_.update(player_);

  auto c = dungeon_.grid_coords(player_.x(), player_.y());
  dungeon_.calculate_visibility(c.first, c.second);

  return true;
}

void DungeonScreen::draw(Graphics& graphics) const {
  const int xo = camera_.xoffset();
  const int yo = camera_.yoffset();

  dungeon_.draw(graphics, xo, yo);
  player_.draw(graphics, xo, yo);
}

Screen* DungeonScreen::next_screen() const {
  return nullptr;
}

std::string DungeonScreen::get_music_track() const {
  return "";
}
