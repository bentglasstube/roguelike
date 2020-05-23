#pragma once

#include <cstdint>
#include <vector>

class Voronoi {
  public:
    Voronoi();

    struct Site {
      float x, y;
      float distance(Site other) const;
      uint32_t color() const;
    };

    void reset();
    void add_point(float x, float y);
    Site get_site(float x, float y) const;

  private:

    std::vector<Site> sites_;
};
