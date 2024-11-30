#pragma once

#include <stdint.h>
#include <windows.h>
#include "macro.h"
#include "arena.h"
#include "cam.h"

global bool wnd_running = true;

typedef BYTE byte;

struct color {
  byte r;
  byte g;
  byte b;
  byte __pad;
};

typedef struct color color_t;

struct wnd {
  HWND wnd;
  HBITMAP __dib;
  BITMAPINFO __bmi;
  color_t *scr;
  HDC __dc, __dib_dc;
  uint32_t w, h;
  int __top, __left;
  float mspf;
  arena_t tmp_alloc, scr_alloc;
  int8_t key_state[0xff];
  v2_t prev_mouse_pos, mouse_pos, delta_mouse_pos;

  LARGE_INTEGER __start, __prev_time, __time, __freq; /* ticks, ticks, ticks/second */
  float dt, t;

  persp_cam_t cam;
};

typedef struct wnd wnd_t;

wnd_t *wnd_new(HINSTANCE inst, int cmd_show);
void wnd_run(wnd_t *w);
