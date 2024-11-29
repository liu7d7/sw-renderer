#include "arena.h"
#include "stdlib.h"
#include "string.h"
#include "err.h"

arena_t 
arena_new(int size) {
  return (arena_t){
    .buf = malloc(size),
    .pos = 0,
    .high_water_mark = 0
  };
}

void*
arena_alloc(arena_t *a, int size) {
  int p = a->pos;
  a->pos += size + 4;
  *(int *)(a->buf + p) = a->pos;
  a->high_water_mark = max(a->high_water_mark, a->pos);
  return a->buf + p + 4;
}

void*
arena_cpy(arena_t *a, int size, void *mem) {
  void *out = arena_alloc(a, size);
  memcpy(out, mem, size);
  return out;
}

void*
arena_alloc_0(arena_t *a, int size) {
  void *out = arena_alloc(a, size);
  memset(out, 0, size);
  return out;
}

void*
arena_alloc_1(arena_t *a, int size) {
  void *out = arena_alloc(a, size);
  memset(out, 0xff, size);
  return out;
}

void
arena_ret(arena_t *a, void *mem) {
  if (*(int *)(mem - 4) == a->pos) {
    a->pos = mem - a->buf - 4;
    return;
  }

  err("can't ret mem; the block was not the most recently alloc'd block");
}

void
arena_reset(arena_t *a) {
  a->pos = 0;
}
