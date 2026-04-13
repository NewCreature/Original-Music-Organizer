#ifndef T3F_GUI_H
#define T3F_GUI_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "t3f.h"

#define T3F_GUI_MAX_ELEMENTS    128

/* GUI element types */
#define T3F_GUI_ELEMENT_TEXT           0 // text element
#define T3F_GUI_ELEMENT_IMAGE          1 // image element
#define T3F_GUI_ELEMENT_ANIMATION      2 // animation element
#define T3F_GUI_ELEMENT_MULTILINE_TEXT 3 // multiline text element

/* GUI_element flags */
#define T3F_GUI_ELEMENT_NO_HOVER  (1 << 0)
#define T3F_GUI_ELEMENT_NO_NAV    (1 << 1)
#define T3F_GUI_ELEMENT_CENTER_H  (1 << 2)
#define T3F_GUI_ELEMENT_CENTER_V  (1 << 3)
#define T3F_GUI_ELEMENT_SHADOW    (1 << 4) // element has shadow
#define T3F_GUI_ELEMENT_AUTOHIDE  (1 << 5) // element is hidden unless mouse pointer is close
#define T3F_GUI_ELEMENT_COPY      (1 << 6) // element maintains its own copy of the data
#define T3F_GUI_ELEMENT_ON_TOUCH  (1 << 7) // active upon touch
#define T3F_GUI_ELEMENT_OWN       (1 << 8)
#define T3F_GUI_ELEMENT_TOP       (1 << 9)
#define T3F_GUI_ELEMENT_SUB       (1 << 10)
#define T3F_GUI_ELEMENT_INITIAL   (1 << 11)
#define T3F_GUI_ELEMENT_BACK      (1 << 12)
#define T3F_GUI_ELEMENT_CENTRE T3F_GUI_ELEMENT_CENTER_H
#define T3F_GUI_ELEMENT_CENTER T3F_GUI_ELEMENT_CENTER_H
#define T3F_GUI_ELEMENT_STATIC (T3F_GUI_ELEMENT_NO_HOVER | T3F_GUI_ELEMENT_NO_NAV)

/* GUI flags */
#define T3F_GUI_DISABLED          (1 << 0) // GUI is disabled
#define T3F_GUI_NO_MOUSE          (1 << 1)
#define T3F_GUI_NO_TOUCH          (1 << 2)
#define T3F_GUI_NO_HIDE           (1 << 3)
#define T3F_GUI_USER_FLAG         (1 << 4)

#define T3F_GUI_INPUT_HOVER 0
#define T3F_GUI_INPUT_NAV   1

typedef struct
{

	int type;
	const void * data;
  void * allocated_data;
	void * resource; // for bitmaps, fonts, etc.
	ALLEGRO_COLOR color;
  ALLEGRO_COLOR inactive_color;
  ALLEGRO_COLOR active_color;
	int flags;
	int (*proc)(void *, int, void *);
	int (*up_proc)(void *, int, void *);
	int (*down_proc)(void *, int, void *);
	int (*left_proc)(void *, int, void *);
	int (*right_proc)(void *, int, void *);
	char * description;
	int description_flags;

	int ox, oy;
	int option;
	int d1, d2, d3, d4;
  float sx, sy;
  float hx, hy;
	int show_element_start;
	int show_element_end;

} T3F_GUI_ELEMENT;

typedef struct
{

	T3F_GUI_ELEMENT element[T3F_GUI_MAX_ELEMENTS];
	void * data;
	int elements;
	int flags;

	int ox, oy;

	int input_mode;
	float hover_y;
	int hover_element;
  int font_margin_top;
  int font_margin_bottom;
  int font_margin_left;
  int font_margin_right;
	int tick;

} T3F_GUI;

typedef struct
{

	void(*get_element_edges)(T3F_GUI * pp, int i, int * left, int * top, int * right, int * bottom);
	void(*render_element)(T3F_GUI * pp, int i, bool hover, int flags);

} T3F_GUI_DRIVER;

void t3f_set_gui_driver(T3F_GUI_DRIVER * dp);
void t3f_set_gui_shadow_color(ALLEGRO_COLOR color);
T3F_GUI * t3f_create_gui(int ox, int oy, void * data);
void t3f_destroy_gui(T3F_GUI * pp);

T3F_GUI_ELEMENT * t3f_add_gui_image_element(T3F_GUI * pp, int (*proc)(void *, int, void *), T3F_BITMAP * bitmap, ALLEGRO_COLOR color, int ox, int oy, int flags);
T3F_GUI_ELEMENT * t3f_add_gui_animation_element(T3F_GUI * pp, int (*proc)(void *, int, void *), T3F_ANIMATION * animation, ALLEGRO_COLOR color, int ox, int oy, int flags);
T3F_GUI_ELEMENT * t3f_add_gui_text_element(T3F_GUI * pp, int (*proc)(void *, int, void *), T3F_FONT * font, ALLEGRO_COLOR color, const char * text, int ox, int oy, int flags);
T3F_GUI_ELEMENT * t3f_add_gui_multiline_text_element(T3F_GUI * pp, int (*proc)(void *, int, void *), T3F_FONT * font, ALLEGRO_COLOR color, const char * text, int ox, int oy, int max_width, int max_lines, int flags);
int t3f_describe_last_gui_element(T3F_GUI * pp, char * text, int flags);
void t3f_set_gui_nav_procs(T3F_GUI * pp, int (*up_proc)(void *, int, void *), int (*down_proc)(void *, int, void *), int (*left_proc)(void *, int, void *), int (*right_proc)(void *, int, void *));
void t3f_set_gui_show_element_range(T3F_GUI * pp, int start_element, int end_element);
void t3f_center_gui(T3F_GUI * pp, float oy, float my);
void t3f_set_gui_shadow(T3F_GUI * pp, float x, float y);
void t3f_set_gui_hover_lift(T3F_GUI * pp, float x, float y);
void t3f_set_gui_element_interaction_colors(T3F_GUI * pp, ALLEGRO_COLOR inactive_color, ALLEGRO_COLOR active_color);
int t3f_get_gui_width(T3F_GUI * pp);
int t3f_get_gui_height(T3F_GUI * pp, float * top);

/* manual GUI navigation */
bool t3f_select_hover_gui_element(T3F_GUI * pp, float x, float y);
void t3f_select_previous_gui_element(T3F_GUI * pp);
void t3f_select_next_gui_element(T3F_GUI * pp);
bool t3f_select_gui_element_by_flags(T3F_GUI * pp, int flags);
bool t3f_select_gui_element_by_text(T3F_GUI * pp, const char * text);
bool t3f_gui_up(T3F_GUI * pp, int flags);
bool t3f_gui_down(T3F_GUI * pp, int flags);
bool t3f_gui_left(T3F_GUI * pp, int flags);
bool t3f_gui_right(T3F_GUI * pp, int flags);
void t3f_activate_selected_gui_element(T3F_GUI * pp);
void t3f_reset_gui_input(T3F_GUI * pp);

/* high level navigation (less flexible) */
bool t3f_process_gui(T3F_GUI * pp, int flags);
void t3f_render_gui(T3F_GUI * pp, int flags);

#ifdef __cplusplus
	}
#endif

#endif
