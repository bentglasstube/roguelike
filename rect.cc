#include "rect.h"

Rect::Rect(double left, double top, double right, double bottom) :
  left(left), top(top), right(right), bottom(bottom) {}

bool Rect::empty() const {
  return right == left || top == bottom;
}

double Rect::width() const {
  return right - left;
}

double Rect::height() const {
  return bottom - top;
}

void Rect::draw(Graphics& graphics, int color, bool filled, int xo, int yo) const {
  if (empty()) return;

  const SDL_Rect r = {
    (int)left - xo,
    (int)top - yo,
    (int)width(),
    (int)height(),
  };
  graphics.draw_rect(&r, color, filled);
}

bool Rect::intersect(const Rect& other) const {
  return !(left > other.right || right < other.left ||
           top > other.bottom || bottom < other.top);
}

std::ostream& operator<<(std::ostream& os, const Rect& rect) {
  os << rect.left << "," << rect.top << "-" << rect.right << "," << rect.bottom;
  return os;
}
