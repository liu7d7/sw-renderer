#pragma once

#include <tgmath.h>
#include <intrin.h>

// row vector
union v2 {
  struct { float x, y; };
  struct { float _00, _01; };
  float e[2];
};

#define v2_s "<%.2f, %.2f>"
#define v2_v(v) v.x, v.y

typedef union v2 v2_t;

inline static float
v2_cross(v2_t a, v2_t b) {
  return a.x * b.y - a.y * b.x;
}

inline static v2_t
v2_add(v2_t a, v2_t b) {
  return (v2_t){a.x + b.x, a.y + b.y};
}

inline static v2_t
v2_sub(v2_t a, v2_t b) {
  return (v2_t){a.x - b.x, a.y - b.y};
}

inline static v2_t
v2_mul_v(v2_t a, v2_t b) {
  return (v2_t){a.x * b.x, a.y * b.y};
}

inline static v2_t
v2_mul(v2_t a, float b) {
  return (v2_t){a.x * b, a.y * b};
}

// row vector
union v3 {
  struct { float x, y, z; };
  struct { float _00, _01, _02; };
  float e[3];
  v2_t xy;
};

#define v3_s "<%.2f, %.2f, %.2f>"
#define v3_v(v) v.x, v.y, v.z
#define v3_nv(v) -v.x, -v.y, -v.z

typedef union v3 v3_t;

static const v3_t v3_0 = (v3_t){};

inline static v3_t 
v3_cross(v3_t a, v3_t b) {
  return (v3_t){a.y * b.z - a.z * b.y, 
            a.z * b.x - a.x * b.z, 
            a.x * b.y - a.y * b.x};
}

inline static float 
v3_dot(v3_t a, v3_t b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
  //return fma(a.x, b.x, fma(a.y, b.y, a.z * b.z));
}

inline static v3_t
v3_mul(v3_t a, float b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
  return a;
}

inline static v3_t
v3_mul_v(v3_t a, v3_t b) {
  a.x *= b.x;
  a.y *= b.y;
  a.z *= b.z;
  return a;
}

inline static float
v3_len_sq(v3_t a) {
  return v3_dot(a, a);
}

inline static float
v3_len(v3_t a) {
  return sqrt(v3_dot(a, a));
}

inline static v3_t 
v3_norm(v3_t a) {
  float length = sqrt(v3_dot(a, a));
  return (v3_t){a.x / length, a.y / length, a.z / length};
}

inline static v3_t
v3_add(v3_t a, v3_t b) {
  return (v3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

inline static v3_t
v3_sub(v3_t a, v3_t b) {
  return (v3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

// row vector
union v4 {
  struct { float x, y, z, w; };
  struct { float _00, _01, _02, _03; };
  float e[4];
  v3_t xyz;
  v2_t xy;
};

typedef union v4 v4_t;

inline static float 
v4_dot(v4_t a, v4_t b) {
  return fma(a.x, b.x, fma(a.y, b.y, fma(a.z, b.z, a.w * b.w)));
}

// row major matrix.
// the *rows* are stored *contiguously*
// so it is a row major matrix
// @todo: is this right?
union m4 {
  struct { 
    // row, col
    float _00, _01, _02, _03,
          _10, _11, _12, _13,
          _20, _21, _22, _23,
          _30, _31, _32, _33;
  };

  float e[4][4];
};

typedef union m4 m4_t;

// @hack: returning the new matrix is nice syntax but slow? this might take an entire cache line!
inline static m4_t 
m4_mul(m4_t a, m4_t b) {
  // row dot column
  a._00 = fma(a._00, b._00, fma(a._01, b._10, fma(a._02, b._20, a._03 * b._30)));
  a._01 = fma(a._00, b._01, fma(a._01, b._11, fma(a._02, b._21, a._03 * b._31)));
  a._02 = fma(a._00, b._02, fma(a._01, b._12, fma(a._02, b._22, a._03 * b._32)));
  a._03 = fma(a._00, b._03, fma(a._01, b._13, fma(a._02, b._23, a._03 * b._33)));

  a._10 = fma(a._10, b._00, fma(a._11, b._10, fma(a._12, b._20, a._13 * b._30)));
  a._11 = fma(a._10, b._01, fma(a._11, b._11, fma(a._12, b._21, a._13 * b._31)));
  a._12 = fma(a._10, b._02, fma(a._11, b._12, fma(a._12, b._22, a._13 * b._32)));
  a._13 = fma(a._10, b._03, fma(a._11, b._13, fma(a._12, b._23, a._13 * b._33)));

  a._20 = fma(a._20, b._00, fma(a._21, b._10, fma(a._22, b._20, a._23 * b._30)));
  a._21 = fma(a._20, b._01, fma(a._21, b._11, fma(a._22, b._21, a._23 * b._31)));
  a._22 = fma(a._20, b._02, fma(a._21, b._12, fma(a._22, b._22, a._23 * b._32)));
  a._23 = fma(a._20, b._03, fma(a._21, b._13, fma(a._22, b._23, a._23 * b._33)));

  a._30 = fma(a._30, b._00, fma(a._31, b._10, fma(a._32, b._20, a._33 * b._30)));
  a._31 = fma(a._30, b._01, fma(a._31, b._11, fma(a._32, b._21, a._33 * b._31)));
  a._32 = fma(a._30, b._02, fma(a._31, b._12, fma(a._32, b._22, a._33 * b._32)));
  a._33 = fma(a._30, b._03, fma(a._31, b._13, fma(a._32, b._23, a._33 * b._33)));

  return a;
}

// row vector * row-major matrix
inline static v4_t
v4_m4_mul(v4_t a, m4_t b) {
  // row of vector dot column of matrix.
  /*return (v4_t){
    fma(a._00, b._00, fma(a._01, b._10, fma(a._02, b._20, a._03 * b._30))),
    fma(a._00, b._01, fma(a._01, b._11, fma(a._02, b._21, a._03 * b._31))),
    fma(a._00, b._02, fma(a._01, b._12, fma(a._02, b._22, a._03 * b._32))),
    fma(a._00, b._03, fma(a._01, b._13, fma(a._02, b._23, a._03 * b._33))),
  };*/
  return (v4_t){
    a._00 * b._00 + a._01 * b._10 + a._02 * b._20 + a._03 * b._30,
    a._01 * b._01 + a._01 * b._11 + a._02 * b._21 + a._03 * b._31,
    a._02 * b._02 + a._01 * b._12 + a._02 * b._22 + a._03 * b._32,
    a._03 * b._03 + a._01 * b._13 + a._02 * b._23 + a._03 * b._33,
  };
}

inline static m4_t
m4_transpose(m4_t a) {
  return (m4_t){
    a._00, a._10, a._20, a._30,
    a._01, a._11, a._21, a._31,
    a._02, a._12, a._22, a._32,
    a._03, a._13, a._23, a._33,
  };
}
