#include "overworld_screen.h"

OverworldScreen::OverworldScreen() : world_(256, 224) {
  world_.generate(8675309);
}

bool OverworldScreen::update(const Input&, Audio&, unsigned int) {
  return true;
}

void OverworldScreen::draw(Graphics& graphics) const {
  world_.draw(graphics);
}

Screen* OverworldScreen::next_screen() const {
  return nullptr;
}
