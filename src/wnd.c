#include "wnd.h"
#include "macro.h"
#include "err.h"
#include "math.h"
#include "rstr.h"
#include <tgmath.h>
#include "vecmath.h"

void
_wnd_dib_resize(wnd_t *w) {
  if (w->__dib_dc) {
    DeleteDC(w->__dib_dc);
  }

  w->__dib_dc = CreateCompatibleDC(NULL);

  BITMAPINFO info = {
    .bmiHeader = {
      .biSize = sizeof(BITMAPINFOHEADER),
      .biBitCount = 32,
      .biWidth = w->w,
      .biHeight = w->h,
      .biPlanes = 1,
      .biCompression = BI_RGB,
    }
  };

  void *scr;
  w->__dib = CreateDIBSection(
      w->__dc,
      &info,
      DIB_RGB_COLORS,
      &scr,
      0, 0);

  w->scr = scr;

  SelectObject(w->__dib_dc, w->__dib);
}

void
_wnd_clear(wnd_t *w, color_t color) {
  for (int i = 0; i < w->w * w->h; i++) {
    w->scr[i] = color;
  }
}

int 
wnd_clamp_x(wnd_t *w, int x) {
  return min(w->w, max(0, x));
}

int 
wnd_clamp_y(wnd_t *w, int y) {
  return min(w->h, max(0, y));
}

v4_t 
_tmp_main(rstr_vsh_t *v, 
          void const *uni, 
          void const *vi, 
          void *vo) {
  v3_t const *in = vi;
  persp_cam_t const *cam = uni;
  v4_t world = {in->x, in->y, in->z, 1};
  v4_t camera = v4_m4_mul(world, cam->view);
  v4_t ndc = v4_m4_mul(camera, cam->proj);
  return ndc;
}

void
_wnd_draw(wnd_t *w) {
  _wnd_clear(w, (color_t){});

  static float a = 0;

  a += w->dt * 100;

  float x = w->t;

  v3_t cube[] = {
    {x, 0, 0},
    {x, 10, 0},
    {x + 10, 10, 0},
    {x + 10, 10, 0},
    {x + 10, 0, 0},
    {x, 0, 0},

    {x, 10, 0},
    {x, 10, 10},
    {x + 10, 10, 10},
    {x + 10, 10, 10},
    {x + 10, 10, 0},
    {x, 10, 0},

    {x, 0, 10},
    {x + 10, 10, 10},
    {x, 10, 10},
    {x + 10, 10, 10},
    {x, 0, 10},
    {x + 10, 0, 10},

    {x, 0, 0},
    {x + 10, 0, 10},
    {x, 0, 10},
    {x + 10, 0, 10},
    {x, 0, 0},
    {x + 10, 0, 0},

    {x + 10, 0, 0},
    {x + 10, 10, 0},
    {x + 10, 0, 10},
    {x + 10, 0, 10},
    {x + 10, 10, 0},
    {x + 10, 10, 10},

    {x, 0, 0},
    {x, 0, 10},
    {x, 10, 0},
    {x, 0, 10},
    {x, 10, 10},
    {x, 10, 0},
  };
  
  v3_t *vi = 
    arena_cpy(
        &w->tmp_alloc, 
        sizeof(cube),
        cube
      );

  rstr_vsh_t vsh = {
    .vi = vi,
    .i_size = sizeof(v3_t),
    .ndc = arena_alloc(&w->tmp_alloc, 36 * sizeof(v3_t)),
    .main = _tmp_main,
    .uni = &w->cam,
    .n_verts = 36
  };

  rstr_vsh(&vsh);
  color_t *to_blit = rstr_test(&vsh, w->w, w->h);
  memcpy(w->scr, to_blit, w->w * w->h * sizeof(color_t));
  free(to_blit);
}

float 
_time_delta(wnd_t *w, 
            LARGE_INTEGER ticks_1, 
            LARGE_INTEGER ticks_2) {
  LARGE_INTEGER elapsed;
  elapsed.QuadPart = ticks_2.QuadPart - ticks_1.QuadPart;
  float dt = (float)((double)elapsed.QuadPart / (double)w->__freq.QuadPart) * 1000;
  return dt;
}

void 
_wnd_update(wnd_t *w, HDC pdc) {
  if (!w->__dib_dc) {
    _wnd_dib_resize(w);
  }

  LARGE_INTEGER elapsed;
  float dms;
  while ((dms = _time_delta(w, w->__prev_time, w->__time)) < w->mspf) {
    QueryPerformanceCounter(&w->__time);
  }

  if (w->__prev_time.QuadPart == 0) {
    w->__prev_time = w->__time;
    w->dt = 0.000001f;
  } else {
    w->dt = dms / 1000.f;
  }

  w->t = _time_delta(w, w->__start, w->__time) / 1000.f;

  bool f, b, l, r, u, d;
  f = b = l = r = u = d = false;
  if (w->key_state[0x57] > 0) f = true;
  if (w->key_state[0x53] > 0) b = true;
  if (w->key_state[0x41] > 0) l = true;
  if (w->key_state[0x44] > 0) r = true;
  if (w->key_state[VK_SHIFT] > 0) d = true;
  if (w->key_state[VK_SPACE] > 0) u = true;

  persp_cam_move(&w->cam, f, b, l, r, u, d, v2_v(w->delta_mouse_pos));
  w->prev_mouse_pos = w->mouse_pos;
  w->delta_mouse_pos = (v2_t){};

  _wnd_draw(w);

  w->__prev_time = w->__time;

  StretchBlt(pdc, 0, 0, w->w, w->h, w->__dib_dc, 0, 0, w->w, w->h, SRCCOPY);
  GdiFlush();

  // @todo: do this on fixed update
  for (int i = 0; i < 0xff; i++) {
    if (w->key_state[i] == -1) {
      w->key_state[i] = 0;
    }
  }
}

void 
_wnd_update_cr(wnd_t *w) {
  RECT cr;
  GetWindowRect(w->wnd, &cr);
  TITLEBARINFO tbi = {
    .cbSize = sizeof(TITLEBARINFO)
  };
  GetTitleBarInfo(w->wnd, &tbi);
  int title_bar_height = tbi.rcTitleBar.bottom - tbi.rcTitleBar.top;
  w->w = cr.right - cr.left;
  w->h = cr.bottom - cr.top - title_bar_height;
  w->__top = cr.top + title_bar_height;
  w->__left = cr.left;
}

LRESULT
_wnd_proc(HWND hwnd, 
         UINT msg, 
         WPARAM wpar, 
         LPARAM lpar) {
  persistent wnd_t *w;

  switch (msg) {
    case WM_CREATE: {
      w = ((CREATESTRUCT *)lpar)->lpCreateParams;
      w->__dc = GetDC(NULL);
      w->wnd = hwnd;
      
      QueryPerformanceCounter(&w->__start);
      QueryPerformanceFrequency(&w->__freq);

      _wnd_update_cr(w);
      persp_cam_update_aspect(&w->cam, (float)w->w / w->h);
      return 0;
    }
    case WM_SIZE: {
      _wnd_update_cr(w);
      _wnd_dib_resize(w);
      return 0;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC pdc = BeginPaint(w->wnd, &ps);
      _wnd_update(w, pdc);
      EndPaint(w->wnd, &ps);
      return 0;
    }
    case WM_KEYDOWN: {
      if (w->key_state[wpar]) {
        w->key_state[wpar] = 2;
      } else {
        w->key_state[wpar] = 1;
      }
      return 0;
    }

    case WM_INPUT: {
      UINT size;
      GetRawInputData((HRAWINPUT)lpar, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
      void *buf = arena_alloc(&w->tmp_alloc, size);

      if (GetRawInputData((HRAWINPUT)lpar, RID_INPUT, buf, &size, sizeof(RAWINPUTHEADER)) != size) {
        err("GetRawInputData returned wrong size");
      }

      RAWINPUT *raw = buf;

      if (raw->header.dwType == RIM_TYPEMOUSE) {
        w->delta_mouse_pos = (v2_t){raw->data.mouse.lLastX, raw->data.mouse.lLastY};
        SetCursorPos(w->__left + w->w / 2, w->__top + w->h / 2);
      }

      arena_ret(&w->tmp_alloc, buf);

      break;
    }
    case WM_KEYUP: {
      w->key_state[wpar] = -1;
      return 0;
    }
    case WM_DESTROY:
      wnd_running = false;
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(hwnd, msg, wpar, lpar);
}

wnd_t*
wnd_new(HINSTANCE inst, int cmd_show) {
  char const *class_name = "kyoto";

  WNDCLASS wnd_class = {
    .hInstance = inst,
    .style = CS_VREDRAW|CS_HREDRAW|CS_OWNDC,
    .lpszClassName = class_name,
    .lpfnWndProc = _wnd_proc,
    .hCursor = LoadCursor(inst, IDC_ARROW)
  };

  if (!RegisterClass(&wnd_class)) {
    err("RegisterClass");
  }

  wnd_t *wp = malloc(sizeof(wnd_t));
  *wp = (wnd_t){
    .mspf = 1.f / 60.f,
    .tmp_alloc = arena_new(4 * 1 << 20),
    .cam = persp_cam_new((v3_t){0, 0, 10}, 
                         (v3_t){0, 1, 0}, 
                         -3.14159 / 2, 0,
                         0.01, 100,
                         3.14159 / 4., 16. / 10.)
  };

  HWND wnd = CreateWindow(
      class_name, "kyoto", 
      WS_VISIBLE|WS_OVERLAPPEDWINDOW, 
      CW_USEDEFAULT, CW_USEDEFAULT, 
      2304, 1440,
      NULL, NULL, 
      inst, wp);

  if (!wp->wnd) {
    err("CreateWindow");
  }

  RAWINPUTDEVICE mouse = {
    .usUsagePage = 0x01,
    .usUsage = 0x02,
    .dwFlags = RIDEV_NOLEGACY,
  };

  if (!RegisterRawInputDevices(&mouse, 1, sizeof(RAWINPUTDEVICE))) {
    err("failed to register mouse");
  }

  ShowCursor(false);
  
  return wp;
}

void
wnd_run(wnd_t *w) {
  MSG msg;
 
  while (wnd_running) {
event:
    if (PeekMessage(&msg, w->wnd, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      goto event;
    } else {
      HDC pdc = GetDC(w->wnd);
      _wnd_update(w, pdc);
      ReleaseDC(w->wnd, pdc);
    }

    arena_reset(&w->tmp_alloc);
  }
}

/*
void
_plot_tri_bounding_line(int *min_x, 
                        int *max_x, 
                        int x0, 
                        int y0, 
                        int x1, 
                        int y1) {
  // see https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

  int dx = abs(x1 - x0);
  int sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0);
  int sy = y0 < y1 ? 1 : -1;
  int error = dx + dy;

  for (;;) {
    min_x[y0] = min(min_x[y0], x0);
    max_x[y0] = max(max_x[y0], x0);

    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * error;

    if (e2 >= dy) {
      error += dy;
      x0 += sx;
    }

    if (e2 <= dx) {
      error += dx;
      y0 += sy;
    }
  }
}

void
_wnd_tri(wnd_t *w,
         arena_t *arena,
         int x1, int y1, 
         int x2, int y2, 
         int x3, int y3, 
         color_t color) {
  int min_x = min(x1, min(x2, x3));
  int max_x = max(x1, max(x2, x3)) + 1;
  int min_y = min(y1, min(y2, y3));
  int max_y = max(y1, max(y2, y3)) + 1;

  y1 -= min_y;
  y2 -= min_y;
  y3 -= min_y;

  int width = max_x - min_x;
  int height = max_y - min_y;

  int *min_x_per_line = arena_alloc_1(arena, height * sizeof(int));
  int *max_x_per_line = arena_alloc_0(arena, height * sizeof(int));

  // first try
  _plot_tri_bounding_line(min_x_per_line, max_x_per_line, x1, y1, x2, y2);
  _plot_tri_bounding_line(min_x_per_line, max_x_per_line, x2, y2, x3, y3);
  _plot_tri_bounding_line(min_x_per_line, max_x_per_line, x3, y3, x1, y1);

  int maxh = min(height, w->h - min_y);
  int maxw = min(width, w->w - min_x);

  for (int i = 0; i < maxh; i++) {
    for (int j = min_x_per_line[i]; j <= max_x_per_line[i]; j++) {
      int a = (i + min_y) * w->w + j;
      w->scr[a] = color;
    }
  }
}
*/
