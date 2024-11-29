#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define err(...) err_impl(__FILE__, __func__, __LINE__, __VA_ARGS__)

_Noreturn static void
err_impl(char const *file, char const *func, int line, char const *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "%s:%s:%d :: ", file, func, line);
  vfprintf(stderr, fmt, args);
  va_end(args);
  exit(-1);
}

#if 1
#define print(...) print_impl(__FILE__, __func__, __LINE__, __VA_ARGS__)
#else
#define print(...)
#endif

static void
print_impl(char const *file, char const *func, int line, char const *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "%s:%s:%d :: ", file, func, line);
  vfprintf(stderr, fmt, args);
  va_end(args);
}
