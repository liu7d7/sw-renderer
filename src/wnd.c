#include "wnd.h"
#include "macro.h"
#include "err.h"
#include "math.h"
#include "rstr.h"
#include <tgmath.h>
#include "vecmath.h"
#include "mesh.h"

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

  w->scr = w->fsh.fo = scr;

  SelectObject(w->__dib_dc, w->__dib);
}

void
_wnd_clear(wnd_t *w, color_t color) {
  for (uint32_t i = 0; i < w->w * w->h; i++) {
    w->scr[i] = color;
  }
}

v4_t 
_tmp_v_main(vsh_t*, 
            void const *uni, 
            void const *vi, 
            void *_vo) {
  vert_t const *in = vi;
  vert_t *vo = _vo;

  persp_cam_t const *cam = uni;
  v4_t world = {in->pos.x, in->pos.y, in->pos.z, 1};
  v4_t camera = v4_m4_mul(world, cam->view);
  v4_t ndc = v4_m4_mul(camera, cam->proj);

  vo->pos = v3_mul(in->pos, 1.f / ndc.z);
  vo->norm = v3_mul(in->norm, 1.f / ndc.z);
  vo->tex = v2_mul(in->tex, 1.f / ndc.z);
  vo->mtl = in->mtl;

  return ndc;
}

struct _tmp_f_uni {
  persp_cam_t *cam;
  mat_t *mats;
};

void
_tmp_f_main(fsh_t*,
            void const *_uni,
            void const *_vo,
            float const *,
            void *_fo) {
  static const v3_t light_dir = {0.57735026919, 0.57735026919, 0.57735026919};

  color_t *fo = _fo;
  vert_t const *vo = _vo;
  struct _tmp_f_uni const *uni = _uni;
  mat_t mat = uni->mats[vo->mtl];

  v3_t color = v3_mul(mat.ka, 0.3);

  float diffuse = max(v3_dot(vo->norm, light_dir), 0);
  color = v3_add(color, v3_mul(mat.kd, diffuse * 0.5));

  v3_t view_dir = v3_norm(v3_sub(uni->cam->pos, vo->pos));
  v3_t reflect_dir = v3_reflect(v3_mul(light_dir, -1), vo->norm);
  float spec = pow(max(v3_dot(view_dir, reflect_dir), 0.0), mat.ns);
  color = v3_add(color, v3_mul(mat.ks, spec * 0.5));

  color.r = min(1, max(0, color.r));
  color.g = min(1, max(0, color.g));
  color.b = min(1, max(0, color.b));

  *fo = (color_t){
    .r = color.r * 255,
    .g = color.g * 255,
    .b = color.b * 255,
  };
}

void
_tmp_interp(void const *_v0, 
            void const *_v1, 
            void const *_v2, 
            float u, 
            float v, 
            float w, 
            float z,
            void *_vo) {
  vert_t const *v0 = _v0;
  vert_t const *v1 = _v1;
  vert_t const *v2 = _v2;
  vert_t *vo = _vo;

  // skip vo.pos because we don't use it.

  vo->pos = 
    v3_mul(
      v3_add(
        v3_mul(v0->pos, u), 
        v3_add(v3_mul(v1->pos, v), v3_mul(v2->pos, w))), 1.f / z);

  vo->norm = 
    v3_mul(
      v3_add(
        v3_mul(v0->norm, u), 
        v3_add(v3_mul(v1->norm, v), v3_mul(v2->norm, w))), 1.f / z);

  vo->tex = 
    v2_mul(
      v2_add(
        v2_mul(v0->tex, u), 
        v2_add(v2_mul(v1->tex, v), v2_mul(v2->tex, w))), 1.f / z);

  vo->mtl = v0->mtl;
}

void
_wnd_draw(wnd_t *w) {
  struct _tmp_f_uni *uni = w->fsh.uni;
  uni->cam = &w->cam;
  uni->mats = w->mesh.mats;

  memset(w->scr, 0, w->w * w->h * sizeof(*w->scr));
  vsh_exec(&w->vsh);
  vsh_to_fsh(&w->fsh, &w->vsh);
  fsh_exec(&w->fsh, &w->vsh);
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
  w->__prev_time = w->__time;

  _wnd_draw(w);

  StretchBlt(pdc, 0, 0, w->__w, w->__h, w->__dib_dc, 0, 0, w->w, w->h, SRCCOPY);
  GdiFlush();

  char *text = arena_alloc(&w->tmp_alloc, 64);
  sprintf_s(text, 64, "kyoto | xyz: "v3_s", yp: <%.2f, %.2f>", v3_v(w->cam.pos), w->cam.yaw, w->cam.pitch);
  SetWindowText(w->wnd, text);

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
  w->__w = cr.right - cr.left;
  w->__h = cr.bottom - cr.top - title_bar_height;
  w->w = w->__w / 2;
  w->h = w->__h / 2;
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
      _wnd_dib_resize(w);
      persp_cam_update_aspect(&w->cam, (float)w->w / w->h);

      mesh_t mesh = w->mesh = mesh_new("res/monkey.obj");

      w->vsh = (vsh_t){
        .vi = mesh.verts,
        .i_size = sizeof(*mesh.verts),
        .o_size = sizeof(*mesh.verts),
        .main = _tmp_v_main,
        .uni = &w->cam,
        .n_verts = mesh.n_verts,
        .arena = &w->scr_alloc
      };

      w->fsh = (fsh_t){
        .arena = &w->scr_alloc,
        .fo = w->scr,
        .o_size = sizeof(*w->scr),
        .w = w->w,
        .h = w->h,
        .main = _tmp_f_main,
        .interp = _tmp_interp,
        .uni = malloc(sizeof(struct _tmp_f_uni))
      };

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
        SetCursorPos(w->__left + w->__w / 2, w->__top + w->__h / 2);
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
    .scr_alloc = arena_new(1 << 28),
    .cam = persp_cam_new((v3_t){10.5, 10.5, 10.5}, 
                         (v3_t){0, 1, 0}, 
                         -2.35, -0.67,
                         0.01, 256,
                         3.14159 / 4., 16. / 10.)
  };

  HWND wnd = CreateWindow(
      class_name, class_name, 
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
    arena_reset(&w->scr_alloc);
  }
}

/*

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
