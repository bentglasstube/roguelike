#include "rect.h"

Rect::Rect(double left, double top, double right, double bottom) :
  left(left), top(top), right(right), bottom(bottom) {}

double Rect::width() const {
  return right - left;
}

double Rect::height() const {
  return bottom - top;
}

void Rect::draw(Graphics& graphics, int color, bool filled, int xo, int yo) const {
  const SDL_Rect r = {
    (int)left - xo,
    (int)top - yo,
    (int)width(),
    (int)height(),
  };
  graphics.draw_rect(&r, color, filled);
}
