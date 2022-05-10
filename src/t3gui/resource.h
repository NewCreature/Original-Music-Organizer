#ifndef T3GUI_RESOURCE_H
#define T3GUI_RESOURCE_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "nine_patch.h"

#define T3GUI_RESOURCE_TYPE_BITMAP       0
#define T3GUI_RESOURCE_TYPE_FONT         1
#define T3GUI_RESOURCE_TYPE_DEFAULT_FONT 2
#define T3GUI_RESOURCE_TYPE_BITMAP_FONT  3

#define T3GUI_MAX_RESOURCES      256
#define T3GUI_MAX_BITMAPS        256
#define T3GUI_MAX_FONTS          256

typedef struct
{

    int type;
    char path[1024];
    void * data;
    int data_i;
    int * data_a;
    ALLEGRO_DISPLAY * display;

} T3GUI_RESOURCE;

bool t3gui_load_font(ALLEGRO_FONT ** fp, const char * fn, int size);
bool t3gui_load_bitmap_font(ALLEGRO_FONT ** fp, const char * fn, int * ranges);
bool t3gui_load_bitmap(NINE_PATCH_BITMAP ** bp, const char * fn);
void t3gui_unload_resources(ALLEGRO_DISPLAY * dp, bool delete);
bool t3gui_reload_resources(ALLEGRO_DISPLAY * dp);
void t3gui_remove_font_reference(ALLEGRO_FONT ** fp);
void t3gui_remove_bitmap_reference(NINE_PATCH_BITMAP ** bp);

#endif
