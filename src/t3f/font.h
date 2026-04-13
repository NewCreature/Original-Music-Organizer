#ifndef T3F_FONT_H
#define T3F_FONT_H

#include "t3f.h"

#define T3F_FONT_MAX_CHARACTERS 256
#define T3F_FONT_ALIGN_RIGHT      1
#define T3F_FONT_ALIGN_CENTER     2

#define T3F_FONT_TYPE_AUTO       -1
#define T3F_FONT_TYPE_ALLEGRO     0
#define T3F_FONT_TYPE_T3F         1
#define T3F_FONT_TYPE_NONE        2

typedef struct
{

	void * (*load)(const char * fn, ALLEGRO_FILE * fp, int option, int flags);
	void (*update)(void * data);
	void (*destroy)(void * font);

	int (*get_text_width)(const void * font, const char * text);
	int (*get_font_height)(const void * font);
	void (*draw_glyph)(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int codepoint);
	int (*get_glyph_width)(const void * font, int codepoint);
	bool (*get_glyph_dimensions)(const void * font, int codepoint, int * bbx, int * bby, int * bbw, int * bbh);
	int (*get_glyph_advance)(const void * font, int codepoint1, int codepoint2);

	void (*draw_text)(const void * font, ALLEGRO_COLOR color, float x, float y, float z, float scale, int flags, char const *text);
	void (*draw_textf)(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char *format, ...);

} T3F_FONT_ENGINE;

typedef struct
{
	/* loading helpers */
	T3F_OBJECT_LOADER * object_loader;

	int type;
	T3F_FONT_ENGINE * engine;
	void * font;

} T3F_FONT;

typedef struct
{

	char * text;
	void * next_line;

} T3F_TEXT_LINE;

typedef struct
{

	T3F_TEXT_LINE * line;

} T3F_TEXT_LINES;

/* used by the resource manager */
void * t3f_load_font_data_with_engine_f(T3F_FONT_ENGINE * engine, const char * fn, ALLEGRO_FILE * fp, int option, int flags);
void * t3f_load_font_with_engine(T3F_FONT_ENGINE * engine, const char * fn, int option, int flags);
void * t3f_load_font_data_f(const char * fn, ALLEGRO_FILE * fp, int type, int option, int flags);
void * t3f_load_font_data(const char * fn, int type, int option, int flags);

/* user functions for loading fonts */
T3F_FONT * t3f_load_font(const char * fn, int type, int option, int flags, bool threaded);

ALLEGRO_FONT * t3f_load_bitmap_font(const char * fn);

bool t3f_update_font(T3F_FONT * fp);

void t3f_destroy_font_data(void * data, int type);
void t3f_destroy_font(T3F_FONT * fp);

float t3f_get_text_width(T3F_FONT * fp, const char * text);
float t3f_get_font_line_height(T3F_FONT * fp);
void t3f_draw_text(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char * text);
void t3f_draw_scaled_text(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, float scale, int flags, const char * text);
void t3f_draw_textf(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char * format, ...);

void t3f_draw_glyph(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, int cp);
int t3f_get_glyph_advance(T3F_FONT * fp, int cp1, int cp2);

bool t3f_init_text_lines(T3F_TEXT_LINES * text_lines);
bool t3f_create_text_lines(T3F_TEXT_LINES * text_lines, T3F_FONT * fp, float w, float tab, const char * text);
void t3f_free_text_lines(T3F_TEXT_LINES * text_lines);
void t3f_draw_text_lines(T3F_TEXT_LINES * lines, T3F_FONT * font, ALLEGRO_COLOR color, float x, float y, float z, float tab);
void t3f_draw_multiline_text(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, float w, float tab, int flags, const char * text);
void t3f_draw_scaled_multiline_text(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, float scale, float w, float tab, int flags, const char * text);
void t3f_draw_multiline_textf(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, float w, float tab, int flags, const char * format, ...);

#endif
