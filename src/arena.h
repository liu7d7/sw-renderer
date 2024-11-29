#pragma once

struct arena {
  void *buf;
  int pos;
  int high_water_mark;
};

typedef struct arena arena_t;

arena_t arena_new(int size);
void *arena_alloc(arena_t *a, int size);
void *arena_cpy(arena_t *a, int size, void *mem);
void *arena_alloc_0(arena_t *a, int size);
void *arena_alloc_1(arena_t *a, int size);
void arena_ret(arena_t *a, void *mem);
void arena_reset(arena_t *a);

