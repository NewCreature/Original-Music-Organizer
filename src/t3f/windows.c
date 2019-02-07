#include "t3f.h"
#include <allegro5/allegro_windows.h>

void t3f_set_windows_icon(const char * name)
{
    HWND window = al_get_win_window_handle(t3f_display);
    HINSTANCE instance = GetModuleHandle(NULL);
    HICON icon = LoadIcon(instance, name);
    SetClassLong(window, GCL_HICON, (LPARAM)icon);
}

static void my_strcpy(char * dest, const char * src, int max)
{
    int i;

    for(i = 0; i < strlen(src) && i < max; i++)
    {
        dest[i] = src[i];
    }
    if(i < max)
    {
        dest[i] = 0;
    }
    dest[max - 1] = 0;
}

char * t3f_windows_text_to_utf8(const char * src, char * dest, int max)
{
    ALLEGRO_USTR * ustr;
    int i;

    ustr = al_ustr_new("");
    if(ustr)
    {
        for(i = 0; i < strlen(src); i++)
        {
            al_ustr_append_chr(ustr, (unsigned char)src[i]);
        }
        my_strcpy(dest, al_cstr(ustr), max);
        al_ustr_free(ustr);
    }
    return dest;
}
