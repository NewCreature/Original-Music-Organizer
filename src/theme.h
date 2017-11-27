#ifndef OMO_THEME_H
#define OMO_THEME_H

#include "t3f/t3f.h"
#include "t3gui/theme.h"

#define OMO_THEME_MAX_BITMAPS     32
#define OMO_THEME_MAX_GUI_THEMES  16
#define OMO_THEME_MAX_TEXTS       32
#define OMO_THEME_MAX_TEXT_LENGTH 32

#define OMO_THEME_BITMAP_PREVIOUS_TRACK 0
#define OMO_THEME_BITMAP_PLAY           1
#define OMO_THEME_BITMAP_PAUSE          2
#define OMO_THEME_BITMAP_STOP           3
#define OMO_THEME_BITMAP_NEXT_TRACK     4
#define OMO_THEME_BITMAP_OPEN           5
#define OMO_THEME_BITMAP_ADD            6

#define OMO_THEME_GUI_THEME_BOX         0
#define OMO_THEME_GUI_THEME_LIST_BOX    1
#define OMO_THEME_GUI_THEME_BUTTON      2
#define OMO_THEME_GUI_THEME_SLIDER      3

typedef struct
{

	ALLEGRO_CONFIG * config;
	ALLEGRO_BITMAP * bitmap[OMO_THEME_MAX_BITMAPS];
	T3GUI_THEME * gui_theme[OMO_THEME_MAX_GUI_THEMES];
	char text[OMO_THEME_MAX_TEXTS][OMO_THEME_MAX_TEXT_LENGTH];

} OMO_THEME;

OMO_THEME * omo_load_theme(const char * fn, int mode, int font_size);
void omo_destroy_theme(OMO_THEME * tp);

#endif
