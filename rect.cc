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
  const Graphics::Point p1 = { (int)left - xo, (int)top - yo };
  const Graphics::Point p2 = { (int)right - xo, (int)bottom - yo };
  graphics.draw_rect(p1, p2, color, filled);
}

bool Rect::intersect(const Rect& other) const {
  return !(left > other.right || right < other.left ||
           top > other.bottom || bottom < other.top);
}

std::ostream& operator<<(std::ostream& os, const Rect& rect) {
  os << rect.left << "," << rect.top << "-" << rect.right << "," << rect.bottom;
  return os;
}
