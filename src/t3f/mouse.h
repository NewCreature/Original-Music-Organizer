#ifndef _T3F_MOUSE_H
#define _T3F_MOUSE_H

#include "t3f.h"

/* internal functions */
bool _t3f_initialize_mouse(void);
void _t3f_uninitialize_mouse(void);
void _t3f_handle_mouse_event(ALLEGRO_EVENT * event);
void _t3f_update_mouse_pos(float translate_x, float scale_x, float translate_y, float scale_y);
double _t3f_get_mouse_warp_time(void);
void _t3f_update_mouse_warp_time(double when);

/* API */
bool t3f_mouse_button_held(int button);
bool t3f_mouse_button_pressed(int button);
bool t3f_use_mouse_button_press(int button);
bool t3f_mouse_button_released(int button);
bool t3f_use_mouse_button_release(int button);
float t3f_get_mouse_x(void);
float t3f_get_mouse_y(void);
int t3f_get_mouse_z(void);
bool t3f_get_mouse_mickeys(int * x, int * y, int * z);
void t3f_set_mouse_xy(float x, float y);
void t3f_clear_mouse_state(void);

#endif