#include "rstr.h"
#include "err.h"
#include "omp.h"

void
vsh_exec(vsh_t *r) {
  int n_verts = r->n_verts;
  void const *_vi = r->vi;
  r->__vo = arena_alloc(r->arena, r->o_size * r->n_verts);
  void *_vo = r->__vo;
  vsh_main main = r->main;
  r->__ndc = arena_alloc(r->arena, sizeof(v3_t) * r->n_verts);
  v3_t *ndc = r->__ndc;
  int i_size = r->i_size, o_size = r->o_size;
  void const *uni = r->uni;

#pragma omp parallel for
  for (int i = 0; i < n_verts; i++) {
    void const *vi = _vi + i * i_size;
    void *vo = _vo + i * o_size;

    v4_t before_w_div = main(r, uni, vi, vo);
    before_w_div.xy = v2_mul(before_w_div.xy, 1. / before_w_div.w);
    before_w_div.z = 1.f / before_w_div.z;
    ndc[i] = before_w_div.xyz;
  }
}

inline float 
_edge_fn(v2_t v0, v2_t v1, v2_t p) {
  // @todo: potential to cache the value of a
  v2_t a = v2_sub(v1, v0);
  v2_t b = v2_sub(p, v0);

  return v2_cross(a, b);
}

void
vsh_to_fsh(fsh_t *f,
           vsh_t *v) {
  uint32_t w = f->w, h = f->h;
  v3_t *ndc = v->__ndc;

  fi_t *fi = arena_alloc_1(f->arena, w * h * sizeof(fi_t));
  float *depth = arena_alloc_0(f->arena, w * h * sizeof(float));

  f->__fi = fi;
  f->__depth = depth;

  for (int i = 0; i < v->n_verts; i += 3) {
    // to remap -1..1 to 0..w
    v3_t mul = {0.5 * w, 0.5 * h, 1};
    v3_t add = {0.5 * w, 0.5 * h, 0};

    v3_t v0 = v3_add(v3_mul_v(ndc[i], mul), add);
    v3_t v1 = v3_add(v3_mul_v(ndc[i + 1], mul), add);
    v3_t v2 = v3_add(v3_mul_v(ndc[i + 2], mul), add);

    float min_y = min(v0.y, min(v1.y, v2.y));
    float max_y = max(v0.y, max(v1.y, v2.y));

    float min_x = min(v0.x, min(v1.x, v2.x));
    float max_x = max(v0.x, max(v1.x, v2.x));

    uint32_t yi = max(0, min_y), yf = min(h - 1, max(0, max_y));
    uint32_t xi = max(0, min_x), xf = min(w - 1, max(0, max_x));

    float area = 1. / _edge_fn(v0.xy, v1.xy, v2.xy);

    for (uint32_t y = yi; y <= yf; y++) {
      for (uint32_t x = xi; x <= xf; x++) {
        v2_t p = {x, y};
        float e01 = _edge_fn(v0.xy, v1.xy, p);       
        float e12 = _edge_fn(v1.xy, v2.xy, p);     
        float e20 = _edge_fn(v2.xy, v0.xy, p);
        // @todo: depth, perspective correct

        bool a = e01 >= 0 & e12 >= 0 & e20 >= 0;

        e01 *= area;
        e12 *= area;
        e20 *= area;
        float d = e01 * v2.z + e12 * v0.z + e20 * v1.z;
        a &= depth[y * w + x] <= d;

        float ndepth[] = {depth[y * w + x], d};
        fi_t nfi[] = {
          fi[y * w + x], 
          (fi_t){
            (uint16_t)i, 
            (uint16_t)(e12 * UINT16_MAX),
            (uint16_t)(e20 * UINT16_MAX),
            (uint16_t)(e01 * UINT16_MAX),
          }
        };

        depth[y * w + x] = ndepth[a];
        fi[y * w + x] = nfi[a];
      }
    }
  }
}

void 
fsh_exec(fsh_t *f, vsh_t *v) {
  uint32_t h = f->h, w = f->w;
  void *fo = f->fo;
  fi_t *fi = f->__fi;
  void *vo = v->__vo;
  int o_size = v->o_size, fo_size = f->o_size;
  fsh_interp interp = f->interp;
  fsh_main main = f->main;
  float *depth = f->__depth;
  void const *uni = f->uni;

#pragma omp parallel for
  for (uint32_t r = 0; r < h; r++) {
    for (uint32_t c = 0; c < w; c++) {
      fi_t i = fi[r * w + c];
      if (i.v0 == UINT16_MAX) {
        continue;
      }

      void *v0 = vo + o_size * i.v0;
      void *v1 = vo + o_size * (i.v0 + 1);
      void *v2 = vo + o_size * (i.v0 + 2);

      float _u = ((float)i.u) / UINT16_MAX;
      float _v = ((float)i.v) / UINT16_MAX;
      float _w = ((float)i.w) / UINT16_MAX;

      uint8_t interped[o_size];
      interp(v0, v1, v2, _u, _v, _w, depth[r * w + c], interped);

      main(f, uni, interped, depth, fo + (r * w + c) * fo_size);
    }
  }
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
