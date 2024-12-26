#include "cam.h"

void 
_persp_cam_update_vecs(persp_cam_t *c) {
  // update vectors
  c->front = v3_norm((v3_t){
    cos(c->pitch) * cos(c->yaw),
    sin(c->pitch),
    cos(c->pitch) * sin(c->yaw)
  });

  c->right = v3_norm(v3_cross(c->front, c->world_up));

  // should both be normalized already so yeah.
  c->up = v3_norm(v3_cross(c->front, c->right));

  v3_t f = c->front;
  v3_t s = c->right;
  v3_t t = c->up;
  v3_t eye = c->pos;

  // @hack: wtf why is the x-axis and not the z-axis inverted
  c->view = (m4_t){
    -s.x, t.x, f.x, 0,
    -s.y, t.y, f.y, 0,
    -s.z, t.z, f.z, 0,
    v3_dot(eye, s), -v3_dot(eye, t), -v3_dot(eye, f), 1
  };
}

void
persp_cam_update_aspect(persp_cam_t *c, float aspect) {
  float fov = tan(c->fovy/2);
  float n = c->z_near;
  float f = c->z_far;

  c->proj = (m4_t){
    1.f / (fov * aspect), 0, 0, 0,
    0, 1.f / fov, 0, 0,
    0, 0, -1 / (-f + n), -1,
    0, 0, n / (-f + n), 0
  };
}

persp_cam_t
persp_cam_new(v3_t pos, v3_t world_up, 
        float radians yaw, float radians pitch, 
        float near, float far, 
        float radians fovy, float aspect) {
  persp_cam_t out = {
    .pos = pos, 
    .world_up = world_up, 
    .yaw = yaw, .pitch = pitch, .fovy = fovy,
    .z_near = near, .z_far = far
  };

  _persp_cam_update_vecs(&out);
  persp_cam_update_aspect(&out, aspect);

  return out;
}

void
persp_cam_move(persp_cam_t *c,
         bool f, bool b, 
         bool l, bool r, 
         bool u, bool d,
         float dx, float dy) {
  v3_t move = {
    r - l,
    u - d,
    f - b,
  };

  if (v3_len_sq(move) > 0.0001) {
    move = v3_norm(move);
  } 

  v3_t pos_delta = {};

  pos_delta = v3_add(pos_delta, v3_mul(c->front, move.z));
  pos_delta = v3_add(pos_delta, v3_mul(c->right, move.x));
  pos_delta = v3_add(pos_delta, v3_mul(c->world_up, move.y));

  c->pos = v3_add(c->pos, v3_mul(pos_delta, 0.03));

  c->pitch -= dy * 0.005;
  c->yaw += dx * 0.005;

  if (c->pitch > 3.141 / 2.) c->pitch = 3.141 / 2.;
  if (c->pitch < -3.141 / 2.) c->pitch = -3.141 / 2.;

  _persp_cam_update_vecs(c);
}
