#include "err.h"
#include "wnd.h"
#include <windows.h> 
#include <omp.h>

int 
WinMain(HINSTANCE inst, 
    HINSTANCE prev_inst, 
    LPSTR cmd_line, 
    int cmd_show) {
  BOOL dpi_aware = SetProcessDPIAware();

  wnd_t *w = wnd_new(inst, cmd_show);
  wnd_run(w);
}
