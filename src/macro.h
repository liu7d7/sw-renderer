#pragma once

#define persistent static
#define global static
#define radians
#define degrees

#define check_vec_resize(name) \
  do { \
    if (n_##name >= c_##name) { \
      c_##name *= 2; \
      name = realloc(name, c_##name * sizeof(*name)); \
    } \
  } while (false); \

#define check_vec_resize_n(name, n) \
  do { \
    if (n_##name + n >= c_##name) { \
      c_##name *= 2; \
      name = realloc(name, c_##name * sizeof(*name)); \
    } \
  } while (false); \


#define DEBUG 1
