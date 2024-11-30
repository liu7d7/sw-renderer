#include "rstr.h"
#include "err.h"

void
rstr_vsh(rstr_vsh_t *r) {
  for (int i = 0; i < r->n_verts; i++) {
    void const *vi = r->vi + i * r->i_size;
    void *vo = r->vo + i * r->o_size;

    v4_t before_w_div = r->main(r, r->uni, vi, vo);
    before_w_div.xy = v2_mul(before_w_div.xy, 1. / before_w_div.w);
    r->ndc[i] = before_w_div.xyz;
  }
}

void
rstr_vsh_to_fsh(rstr_vsh_t *v, 
                rstr_fsh_t *f, 
                int w, int h, 
                rstr_interp interp) {

}

inline float 
_edge_fn(v2_t v0, v2_t v1, v2_t p) {
  // @todo: potential to cache the value of a
  v2_t a = v2_sub(v1, v0);
  v2_t b = v2_sub(p, v0);

  return v2_cross(a, b);
}

void
_plot_tri_bounding_line(uint32_t *min_x, 
                        uint32_t *max_x, 
                        int x0, 
                        int y0, 
                        int x1, 
                        int y1) {
  // see https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

  int dx = abs(x1 - x0);
  int sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0);
  int sy = y0 < y1 ? 1 : -1;
  int error = dx + dy;

  for (;;) {
    min_x[y0] = min(min_x[y0], (uint32_t)max(0, (int)x0));
    max_x[y0] = max(max_x[y0], (uint32_t)max(0, (int)x0));

    int e2 = 2 * error;

    if (e2 >= dy) {
      error += dy;
      x0 += sx;
    }

    if (e2 <= dx) {
      error += dx;
      y0 += sy;
    }

    if (x0 == x1 && y0 == y1) break;
  }
}

color_t*
rstr_test(rstr_vsh_t *v, arena_t *arena, uint32_t w, uint32_t h) {
  color_t *color = arena_alloc_0(arena, w * h * sizeof(color_t));
  float *depth = arena_alloc_0(arena, w * h * sizeof(float));
  for (uint32_t i = 0; i < w * h; i++) {
    depth[i] = HUGE_VAL;
  }

  for (int i = 0; i < v->n_verts; i += 3) {
    // to remap -1..1 to 0..w
    v3_t mul = {0.5 * w, 0.5 * h, 1};
    v3_t add = {0.5 * w, 0.5 * h, 0};

    v3_t v0 = v3_add(v3_mul_v(v->ndc[i], mul), add);
    v3_t v1 = v3_add(v3_mul_v(v->ndc[i + 1], mul), add);
    v3_t v2 = v3_add(v3_mul_v(v->ndc[i + 2], mul), add);

    float min_y = min(v0.y, min(v1.y, v2.y));
    float max_y = max(v0.y, max(v1.y, v2.y));

    float min_x = min(v0.x, min(v1.x, v2.x));
    float max_x = max(v0.x, max(v1.x, v2.x));

    uint32_t yi = max(0, min_y), yf = min(h, max(0, max_y));
    uint32_t xi = max(0, min_x), xf = min(w, max(0, max_x));

    float area = 1. / _edge_fn(v0.xy, v1.xy, v2.xy);

#pragma omp parallel for
    for (uint32_t y = yi; y < yf; y++) {
      for (uint32_t x = xi; x < xf; x++) {
        v2_t p = {x + 0.5, y + 0.5};
        float e01 = _edge_fn(v0.xy, v1.xy, p);       
        float e12 = _edge_fn(v1.xy, v2.xy, p);     
        float e20 = _edge_fn(v2.xy, v0.xy, p);
        // @todo: depth, perspective correct

        if (e01 >= 0 && e12 >= 0 && e20 >= 0) {
          e01 *= area;
          e12 *= area;
          e20 *= area;
          float d = e01 * v2.z + e12 * v0.z + e20 * v1.z;
          if (depth[y * w + x] >= d) {
            depth[y * w + x] = d;
            color[y * w + x] = (color_t){255 * d, 255 * d, 255 * d};
          }
        }
      }
    }
  }

  return color;
}
