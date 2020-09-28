#pragma once

#include <cstdint>
#include <cmath>
#include <set>
#include <vector>

#include "graphics.h"

#include "jc_voronoi.h"

class Voronoi {
  public:
    Voronoi();

    void reset();
    void add_point(float x, float y);
    void relax();
    void generate();

    void draw_cell_borders(Graphics& graphics) const;

  private:

    std::vector<jcv_point> points_;
    jcv_diagram diagram_;
    bool generated_;

    void invalidate();
};
