#include "title_screen.h"

#include "overworld_screen.h"

TitleScreen::TitleScreen() : text_("text.png"), backdrop_("title.png") {}

bool TitleScreen::update(const Input& input, Audio&, unsigned int elapsed) {
  timer_ = (timer_ + elapsed) % 1000;
  return !input.any_pressed();
}

void TitleScreen::draw(Graphics& graphics) const {
  backdrop_.draw(graphics);
  if (timer_ < 500) {
    const int x = graphics.width() / 2;
    const int y = graphics.height() * 3 / 4;
    text_.draw(graphics, "Press any key", x, y, Text::Alignment::Center);
  }
}

Screen* TitleScreen::next_screen() const {
  return new OverworldScreen();
}

std::string TitleScreen::get_music_track() const {
  return "";
}
