#include "voronoi.h"

#include <assert.h>
#include <limits>

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

#ifndef NDEBUG
#include <iostream>
#endif

Voronoi::Voronoi() {
  reset();
}

void Voronoi::reset() {
  points_.clear();
  invalidate();
}

void Voronoi::add_point(float x, float y) {
  points_.push_back({x, y});
  invalidate();
}

void Voronoi::relax() {
  assert(generated_);

  const jcv_site* sites = jcv_diagram_get_sites(&diagram_);
  for (int i = 0; i < diagram_.numsites; ++i) {
    const jcv_site* site = &sites[i];

    float x = 0, y = 0, a = 0;
    const jcv_graphedge* edge = site->edges;
    while (edge) {
      const jcv_point p1 = edge->pos[0];
      const jcv_point p2 = edge->pos[1];

      const float v = p1.x * p2.y - p2.x * p1.y;
      x += (p1.x + p2.x) * v;
      y += (p1.y + p2.y) * v;
      a += p1.x * p2.y - p1.y * p2.x;

      edge = edge->next;
    }

    points_[site->index].x = x / a / 3.0f;
    points_[site->index].y = y / a / 3.0f;
  }

  generated_ = false;
  generate();
}

void Voronoi::generate() {
  if (generated_) return;

  memset(&diagram_, 0, sizeof(jcv_diagram));
  jcv_diagram_generate(points_.size(), points_.data(), 0, 0, &diagram_);
  generated_ = true;
}

void Voronoi::add_noise(int seed) {
  terrain_.reserve(diagram_.numsites);

  const jcv_site* sites = jcv_diagram_get_sites(&diagram_);
  for (int i = 0; i < diagram_.numsites; ++i) {
    const jcv_site* site = &sites[i];
    const float cx = site->p.x;
    const float cy = site->p.y;

    // TODO generate terrain information for each site
    const float height = stb_perlin_ridge_noise3(cx, cy, seed, 2.0, 0.5, 1.0, 6);
    const float temp = 0;
    const float moisture = 0;

    terrain_.push_back({ height, temp, moisture });
  }
}

void Voronoi::draw_cell_borders(Graphics& graphics) const {
  assert(generated_);
  const jcv_edge* edge = jcv_diagram_get_edges(&diagram_);
  while (edge) {
    const Graphics::Point p1 = { (int)edge->pos[0].x, (int)edge->pos[0].y };
    const Graphics::Point p2 = { (int)edge->pos[1].x, (int)edge->pos[1].y };
    graphics.draw_line(p1, p2, 0xd8ff00ff);
    edge = jcv_diagram_get_next_edge(edge);
  }
}

void Voronoi::invalidate() {
  memset(&diagram_, 0, sizeof(jcv_diagram));
  generated_ = false;
}
