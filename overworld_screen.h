#pragma once

#include "screen.h"

#include "overworld.h"

class OverworldScreen : public Screen {
  public:

    OverworldScreen();
    bool update(const Input& input, Audio& audio, unsigned int elapsed) override;
    void draw(Graphics& graphics) const override;

    Screen* next_screen() const override;

  private:
    Overworld world_;
};
