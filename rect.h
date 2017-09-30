#pragma once

#include "graphics.h"

class Rect {
  public:
    Rect(double left, double top, double right, double bottom);
    double left, top, right, bottom;

    double width() const;
    double height() const;

    void draw(Graphics& graphics, int color, bool filled, int xo, int yo) const;
};