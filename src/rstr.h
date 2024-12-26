#pragma once

#include "vecmath.h"
#include "stdint.h"
#include "arena.h"

// @todo: get rid of data interleaving, hard to express in C. 

typedef struct vsh vsh_t;

typedef v4_t (*vsh_main)(vsh_t *r, void const *uni, void const *vi, void *vo);

struct vsh {
  void *vi;
  void *uni;
  void *__vo;
  v3_t *__ndc;
  int i_size, o_size;
  int n_verts;
  vsh_main main;
  arena_t *arena;
};

void vsh_exec(vsh_t *r);

typedef struct fi fi_t;

struct fi {
  uint16_t v0;
  uint16_t u;
  uint16_t v;
  uint16_t w;
} __attribute__((packed));

typedef struct fsh fsh_t;

typedef void (*fsh_main)(fsh_t *f, void const *uni, void const *vo, float const *depth, void *fo);
typedef void (*fsh_interp)(void const *v0, void const *v1, void const *v2, float u, float v, float w, float z, void *vo);

struct fsh {
  fi_t *__fi;
  float *__depth;
  void *fo;
  void *uni;
  int o_size, __v_size;
  uint32_t w, h;
  fsh_interp interp;
  arena_t *arena;
  fsh_main main;
};

void vsh_to_fsh(fsh_t *f, vsh_t *v);

void fsh_exec(fsh_t *f, vsh_t *v);
