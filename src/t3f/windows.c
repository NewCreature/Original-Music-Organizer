#include "t3f.h"
#include <allegro5/allegro_windows.h>

void t3f_set_windows_icon(const char * name)
{
    HWND window = al_get_win_window_handle(t3f_display);
    HINSTANCE instance = GetModuleHandle(NULL);
    HICON icon = LoadIcon(instance, name);
    SetClassLong(window, GCL_HICON, (LPARAM)icon);
}
