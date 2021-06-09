#ifndef T3F_GUI_H
#define T3F_GUI_H

#ifdef __cplusplus
   extern "C" {
#endif

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>

#define T3F_GUI_MAX_ELEMENTS    128

/* GUI element types */
#define T3F_GUI_ELEMENT_TEXT      0 // text element
#define T3F_GUI_ELEMENT_IMAGE     1 // image element

/* GUI_element flags */
#define T3F_GUI_ELEMENT_STATIC    1 // do not animate on hover
#define T3F_GUI_ELEMENT_CENTRE    2 // element is centered
#define T3F_GUI_ELEMENT_SHADOW    4 // element has shadow
#define T3F_GUI_ELEMENT_AUTOHIDE  8 // element is hidden unless mouse pointer is close
#define T3F_GUI_ELEMENT_COPY     16 // element maintains its own copy of the data
#define T3F_GUI_ELEMENT_ON_TOUCH 32 // active upon touch

/* GUI flags */
#define T3F_GUI_DISABLED          1 // GUI is disabled

typedef struct
{

	int type;
	const void * data;
  void * allocated_data;
	void ** resource; // for bitmaps, fonts, etc.
	ALLEGRO_COLOR color;
  ALLEGRO_COLOR inactive_color;
  ALLEGRO_COLOR active_color;
	int flags;
	int (*proc)(void *, int, void *);
	char * description;

	int ox, oy;
	int d1, d2, d3, d4;
  float sx, sy;
  float hx, hy;

} T3F_GUI_ELEMENT;

typedef struct
{

	T3F_GUI_ELEMENT element[T3F_GUI_MAX_ELEMENTS];
	int elements;
	int flags;

	int ox, oy;

	int hover_element;
  int font_margin_top;
  int font_margin_bottom;
  int font_margin_left;
  int font_margin_right;

} T3F_GUI;

typedef struct
{

	float(*get_element_width)(T3F_GUI_ELEMENT * ep);
	float(*get_element_height)(T3F_GUI_ELEMENT * ep);
	void(*render_element)(T3F_GUI * pp, int i, bool hover);

} T3F_GUI_DRIVER;

void t3f_set_gui_driver(T3F_GUI_DRIVER * dp);
T3F_GUI * t3f_create_gui(int ox, int oy);
void t3f_destroy_gui(T3F_GUI * pp);

int t3f_add_gui_image_element(T3F_GUI * pp, int (*proc)(void *, int, void *), void ** bp, int ox, int oy, int flags);
int t3f_add_gui_text_element(T3F_GUI * pp, int (*proc)(void *, int, void *), const char * text, void ** fp, int ox, int oy, ALLEGRO_COLOR color, int flags);
int t3f_describe_last_gui_element(T3F_GUI * pp, char * text);
void t3f_center_gui(T3F_GUI * pp, float oy, float my);
void t3f_set_gui_shadow(T3F_GUI * pp, float x, float y);
void t3f_set_gui_hover_lift(T3F_GUI * pp, float x, float y);
void t3f_set_gui_element_interaction_colors(T3F_GUI * pp, ALLEGRO_COLOR inactive_color, ALLEGRO_COLOR active_color);
int t3f_get_gui_width(T3F_GUI * pp);
int t3f_get_gui_height(T3F_GUI * pp, float * top);

void t3f_select_previous_gui_element(T3F_GUI * pp);
void t3f_select_next_gui_element(T3F_GUI * pp);
void t3f_activate_selected_gui_element(T3F_GUI * pp, void * data);
void t3f_process_gui(T3F_GUI * pp, void * data);
void t3f_render_gui(T3F_GUI * pp);

#ifdef __cplusplus
	}
#endif

#endif
