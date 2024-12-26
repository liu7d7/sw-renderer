#pragma once

#include "vecmath.h"

typedef struct vert vert_t;

struct vert {
  v3_t pos;
  v3_t norm;
  v2_t tex;
  int mtl;
};

typedef struct mat mat_t;

struct mat {
  char const *name;
  v3_t ka, kd, ks;
  float ns;
};

typedef struct mesh mesh_t;

struct mesh {
  vert_t *verts;
  int n_verts;

  mat_t *mats;
  int n_mats;
};

mesh_t mesh_new(char const *file);
