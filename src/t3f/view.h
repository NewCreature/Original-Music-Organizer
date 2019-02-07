#ifndef T3F_VIEW_H
#define T3F_VIEW_H

#include <allegro5/allegro5.h>

/* structure holds information about a 3D viewport usually used to represent
   one player's screen, split screen games will have multiple viewports */
typedef struct
{

	/* virtual dimensions */
	int virtual_width;
	int virtual_height;

	/* offset of viewport */
	float offset_x;
	float offset_y;
	float width;
	float height;

	/* vanishing point */
	float vp_x;
	float vp_y;

	int flags;

	bool need_update;
	float translate_x;
	float translate_y;
	float scale_x;
	float scale_y;
	ALLEGRO_TRANSFORM transform;

	/* edges */
	float top;
	float bottom;
	float left;
	float right;

} T3F_VIEW;

extern T3F_VIEW * t3f_default_view;
extern T3F_VIEW * t3f_current_view;

T3F_VIEW * t3f_create_view(float ox, float oy, float w, float h, float vpx, float vpy, int flags);
void t3f_set_view_virtual_dimensions(T3F_VIEW * vp, int w, int h);
void t3f_adjust_view(T3F_VIEW * vp, float ox, float oy, float w, float h, float vpx, float vpy, int flags);
void t3f_destroy_view(T3F_VIEW * vp);
void t3f_select_view(T3F_VIEW * sp);
T3F_VIEW * t3f_get_current_view(void);
bool t3f_project_coordinates(float vw, float vpx, float vpy, float * x, float * y, float z);
float t3f_project_x(float x, float z);
float t3f_project_y(float y, float z);
void t3f_select_input_view(T3F_VIEW * vp);

#endif
