#ifndef _T3F_TOUCH_H
#define _T3F_TOUCH_H

#include "t3f.h"

#define T3F_MAX_TOUCHES (ALLEGRO_TOUCH_INPUT_MAX_TOUCH_COUNT + 2)

bool _t3f_initialize_touch_data(void);
void _t3f_uninitialize_touch_data(void);
bool _t3f_initialize_touch(void);
void _t3f_uninitialize_touch(void);
void _t3f_handle_touch_event(ALLEGRO_EVENT * event);
void _t3f_update_real_touch_pos(int point, int x, int y);
void _t3f_update_touch_pos(int point, float translate_x, float scale_x, float translate_y, float scale_y);

bool t3f_touch_active(int point);
bool t3f_touch_pressed(int point);
bool t3f_use_touch_press(int point);
bool t3f_touch_released(int point);
bool t3f_use_touch_release(int point);
float t3f_get_touch_x(int point);
float t3f_get_touch_y(int point);
void t3f_clear_touch_state(void);

#endif
