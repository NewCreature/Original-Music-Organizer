#ifndef T3F_FONT_H
#define T3F_FONT_H

#include "t3f.h"

#define T3F_FONT_MAX_CHARACTERS 256
#define T3F_FONT_OUTLINE          1
#define T3F_FONT_ALIGN_RIGHT      1
#define T3F_FONT_ALIGN_CENTER     2

typedef struct
{
	
	ALLEGRO_BITMAP * bitmap;
	int x, y, width, height;
	
} T3F_FONT_CHARACTER;

typedef struct
{
	
	ALLEGRO_BITMAP * character_sheet;
	T3F_FONT_CHARACTER character[T3F_FONT_MAX_CHARACTERS];
	float adjust;
	float scale;
	
} T3F_FONT;

typedef struct
{
	
	char text[256];
	
} T3F_TEXT_LINE;

typedef struct
{
	
	T3F_FONT * font;
	T3F_TEXT_LINE line[64];
	int lines;
	float tab;
	
} T3F_TEXT_LINE_DATA;

ALLEGRO_FONT * t3f_load_bitmap_font(const char * fn);

T3F_FONT * t3f_generate_font_f(ALLEGRO_FILE * fp, int size, int flags);
T3F_FONT * t3f_generate_font(const char * fn, int size, int flags);
T3F_FONT * t3f_load_font_f(ALLEGRO_FILE * fp, int flags);
T3F_FONT * t3f_load_font(const char * fn, int flags);
bool t3f_save_font(T3F_FONT * fp, const char * fn);
void t3f_destroy_font(T3F_FONT * fp);
float t3f_get_text_width(T3F_FONT * fp, const char * text);
float t3f_get_font_line_height(T3F_FONT * fp);
void t3f_draw_text(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, float w, float tab, int flags, const char * text);
void t3f_draw_textf(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, float w, float tab, int flags, const char * format, ...);

void t3f_create_text_line_data(T3F_TEXT_LINE_DATA * lp, T3F_FONT * fp, float w, float tab, const char * text);
void t3f_draw_text_lines(T3F_TEXT_LINE_DATA * lines, ALLEGRO_COLOR color, float x, float y, float z);

#endif
