#pragma once

#include "player.h"

class Camera {
  public:
    Camera();

    void update(const Player& player);
    int xoffset() const;
    int yoffset() const;

  private:
    static constexpr int kWidth = 256;
    static constexpr int kHeight = 240;

    double ox_, oy_;
};
