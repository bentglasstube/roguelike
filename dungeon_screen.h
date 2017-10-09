#pragma once

#include "audio.h"
#include "backdrop.h"
#include "graphics.h"
#include "input.h"
#include "screen.h"
#include "text.h"

#include "camera.h"
#include "dungeon_set.h"
#include "player.h"

class DungeonScreen : public Screen {
  public:

    DungeonScreen();

    bool update(const Input& input, Audio& audio, unsigned int elapsed) override;
    void draw(Graphics& graphics) const override;

    Screen* next_screen() const override;
    std::string get_music_track() const override;

  private:

    enum class State { FadeIn, Playing, Pause, FadeOut };

    static constexpr int kHudHeight = 48;
    static constexpr int kMapHeight = kHudHeight;
    static constexpr int kMapWidth = kMapHeight * 4/3;
    static constexpr int kFadeTimer = 1000;

    Text text_;
    Camera camera_;
    DungeonSet dungeon_set_;
    Player player_;
    State state_;
    bool take_stairs_;
    int timer_;

    void move_player_to_tile(Dungeon::Tile tile);
};
