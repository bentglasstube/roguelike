#pragma once

#include "player.h"

class Camera {
  public:
    Camera();

    void update(const Player& player);
    int xoffset() const;
    int yoffset() const;

  private:

    double ox_, oy_;
};
