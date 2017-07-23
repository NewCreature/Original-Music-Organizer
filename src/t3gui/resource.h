#ifndef T3GUI_RESOURCE_H
#define T3GUI_RESOURCE_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "nine_patch.h"

#define T3GUI_RESOURCE_TYPE_BITMAP 0
#define T3GUI_RESOURCE_TYPE_FONT   1

#define T3GUI_MAX_RESOURCES      256
#define T3GUI_MAX_BITMAPS        256
#define T3GUI_MAX_FONTS          256

typedef struct
{

    int type;
    char path[1024];
    void * data;
    int data_i;

} T3GUI_RESOURCE;

bool t3gui_load_font(ALLEGRO_FONT ** fp, const char * fn, int size);
bool t3gui_load_bitmap(NINE_PATCH_BITMAP ** bp, const char * fn);
void t3gui_unload_resources(void);
bool t3gui_reload_resources(void);
void t3gui_free_resources(void);

#endif
