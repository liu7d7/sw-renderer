#pragma once

#include "vecmath.h"
#include "wnd.h"

// @todo: get rid of data interleaving, hard to express in C. 

typedef struct rstr_vsh rstr_vsh_t;

typedef v4_t (*rstr_vsh_main)(rstr_vsh_t *r, void const *uni, void const *vi, void *vo);
typedef void (*rstr_interp)(void const *vi0, void const *vi1, void const *vi2, float u, float v, float w, void *vo);

struct rstr_vsh {
  void const *vi;
  void const *uni;
  void *vo;
  v3_t *ndc;
  int i_size, o_size;
  int n_verts;
  rstr_vsh_main main;
};

void rstr_vsh(rstr_vsh_t *r);

typedef struct rstr_fsh rstr_fsh_t;

struct rstr_fsh {
  void const *fi;
  void *fo;
  void *uni;
  int i_size, o_size;
  int w, h;
};

void rstr_vsh_to_fsh(rstr_vsh_t *v, rstr_fsh_t *f, int w, int h, rstr_interp interp);

// @todo: make caller pass in the buffer
color_t *rstr_test(rstr_vsh_t *v, arena_t *arena, uint32_t w, uint32_t h);
