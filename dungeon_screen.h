#pragma once

#include "audio.h"
#include "backdrop.h"
#include "graphics.h"
#include "input.h"
#include "screen.h"
#include "text.h"

#include "camera.h"
#include "dungeon.h"
#include "player.h"

class DungeonScreen : public Screen {
  public:

    DungeonScreen();

    bool update(const Input& input, Audio& audio, unsigned int elapsed) override;
    void draw(Graphics& graphics) const override;

    Screen* next_screen() const override;
    std::string get_music_track() const override;

  private:

    static constexpr int kHudHeight = 48;

    Text text_;
    Camera camera_;
    Dungeon dungeon_;
    Player player_;
};
