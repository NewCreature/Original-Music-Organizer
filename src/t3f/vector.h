#ifndef T3F_VECTOR_H
#define T3F_VECTOR_H

#ifdef __cplusplus
   extern "C" {
#endif

#include <allegro5/allegro5.h>

#define T3F_VECTOR_OBJECT_MAX_SEGMENTS 256
#define T3F_VECTOR_FONT_MAX_CHARACTERS 256

typedef struct
{

	float x, y, z;
	
} T3F_VECTOR_POINT;

typedef struct
{

	T3F_VECTOR_POINT point[2];
	ALLEGRO_COLOR color;
	float thickness;
	
} T3F_VECTOR_SEGMENT;

typedef struct
{

	T3F_VECTOR_SEGMENT * segment[T3F_VECTOR_OBJECT_MAX_SEGMENTS];
	int segments;
	
} T3F_VECTOR_OBJECT;

typedef struct
{
	
	T3F_VECTOR_OBJECT * object;
	float width;
	
} T3F_VECTOR_FONT_CHARACTER;

typedef struct
{
	
	T3F_VECTOR_FONT_CHARACTER * character[T3F_VECTOR_FONT_MAX_CHARACTERS];
	float height;
	
} T3F_VECTOR_FONT;

/* vector object creation */
T3F_VECTOR_OBJECT * t3f_create_vector_object(void);
void t3f_destroy_vector_object(T3F_VECTOR_OBJECT * vp);
bool t3f_add_vector_segment(T3F_VECTOR_OBJECT * vp, float sx, float sy, float sz, float ex, float ey, float ez, ALLEGRO_COLOR color, float thickness);
bool t3f_remove_vector_segment(T3F_VECTOR_OBJECT * vp, unsigned int segment);

/* vector font creation */
T3F_VECTOR_FONT * t3f_create_vector_font(void);
void t3f_destroy_vector_font(T3F_VECTOR_FONT * vfp);
bool t3f_add_vector_character(T3F_VECTOR_FONT * vfp, unsigned int character, T3F_VECTOR_OBJECT * vp, float width);
bool t3f_remove_vector_character(T3F_VECTOR_FONT * vp, unsigned int character);

/* vector object IO */
T3F_VECTOR_OBJECT * t3f_load_vector_object_f(ALLEGRO_FILE * fp);
T3F_VECTOR_OBJECT * t3f_load_vector_object(const char * fn);
bool t3f_save_vector_object_f(T3F_VECTOR_OBJECT * vp, ALLEGRO_FILE * fp);
bool t3f_save_vector_object(T3F_VECTOR_OBJECT * vp, const char * fn);

/* vector font IO */
T3F_VECTOR_FONT * t3f_load_vector_font_f(ALLEGRO_FILE * fp);
T3F_VECTOR_FONT * t3f_load_vector_font(const char * fn);
bool t3f_save_vector_font_f(T3F_VECTOR_FONT * vp, ALLEGRO_FILE * fp);
bool t3f_save_vector_font(T3F_VECTOR_FONT * vp, const char * fn);

/* vector font utility */
float t3f_get_vector_text_width(T3F_VECTOR_FONT * vfp, const char * text);
float t3f_get_morphed_vector_text_width(T3F_VECTOR_FONT * vfp, float sx, const char * text);
float t3f_get_vector_text_height(T3F_VECTOR_FONT * vfp);
float t3f_get_morphed_vector_text_height(T3F_VECTOR_FONT * vfp, float sy);

/* vector object rendering */
void t3f_draw_vector_object(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float tscale);
void t3f_draw_tinted_vector_object(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float tscale, ALLEGRO_COLOR color);
void t3f_draw_morphed_vector_object(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float sx, float sy, float sz, float tscale);
void t3f_draw_morphed_vector_object_extrusion(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float sx, float sy, float sz, float lz, float tscale);
void t3f_draw_tinted_morphed_vector_object(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float sx, float sy, float sz, float tscale, ALLEGRO_COLOR color);
void t3f_draw_tinted_morphed_vector_object_extrusion(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float sx, float sy, float sz, float lz, float tscale, ALLEGRO_COLOR color);

/* vector text rendering */
void t3f_draw_vector_text(T3F_VECTOR_FONT * vfp, ALLEGRO_COLOR color, float x, float y, float z, float tscale, const char * text);
void t3f_draw_morphed_vector_text(T3F_VECTOR_FONT * vfp, ALLEGRO_COLOR color, float x, float y, float z, float sx, float sy, float sz, float tscale, const char * text);
void t3f_draw_morphed_vector_text_extrusion(T3F_VECTOR_FONT * vfp, ALLEGRO_COLOR color, float x, float y, float z, float sx, float sy, float sz, float lz, float tscale, const char * text);

#ifdef __cplusplus
	}
#endif

#endif
