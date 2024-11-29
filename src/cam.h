#pragma once

#include "vecmath.h"
#include "macro.h"

struct persp_cam {
  v3_t pos, front, right, up, world_up;
  float z_near, z_far;
  float radians yaw, pitch, fovy;
  m4_t proj, view;
};

typedef struct persp_cam persp_cam_t;

persp_cam_t persp_cam_new(v3_t pos, v3_t world_up, 
                          float yaw, float pitch, 
                          float near, float far, 
                          float radians fovy, float aspect);

void persp_cam_move(persp_cam_t *c,
              bool f, bool b, 
              bool l, bool r, 
              bool u, bool d,
              float dx, float dy);

void persp_cam_update_aspect(persp_cam_t *c, float aspect);

m4_t cam_view(persp_cam_t *c);
