#ifndef T3GUI_THEME_H
#define T3GUI_THEME_H

#include "defines.h"
#include "nine_patch.h"

#define T3GUI_THEME_MAX_COLORS          4
#define T3GUI_THEME_MAX_BITMAPS         4
#define T3GUI_THEME_MAX_FONTS           4
#define T3GUI_THEME_MAX_SOURCE_BITMAPS 32

#define T3GUI_THEME_COLOR_FG    0
#define T3GUI_THEME_COLOR_BG    1
#define T3GUI_THEME_COLOR_MG    2
#define T3GUI_THEME_COLOR_EG    3

typedef struct
{

    NINE_PATCH_BITMAP * bitmap[T3GUI_THEME_MAX_BITMAPS];
    ALLEGRO_COLOR color[T3GUI_THEME_MAX_BITMAPS];
    ALLEGRO_FONT * font[T3GUI_THEME_MAX_FONTS];
    void * aux_font; // alternate font system
    int left_margin;
    int right_margin;
    int top_margin;
    int bottom_margin;
    int scrollbar_size;
    int min_space;
    int click_travel;

} T3GUI_THEME_STATE;

typedef struct
{

    T3GUI_THEME_STATE state[T3GUI_ELEMENT_STATES];
    NINE_PATCH_BITMAP * bitmap[T3GUI_THEME_MAX_SOURCE_BITMAPS];
    char bitmap_path[T3GUI_THEME_MAX_SOURCE_BITMAPS][1024];

} T3GUI_THEME;

T3GUI_THEME * t3gui_get_default_theme(void);
T3GUI_THEME * t3gui_load_theme(const char * fn, int font_size);
void t3gui_destroy_theme(T3GUI_THEME * tp);

#endif
