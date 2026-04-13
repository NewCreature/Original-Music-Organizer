#ifndef _T3F_KEYBOARD_H
#define _T3F_KEYBOARD_H

#include "t3f.h"

#define T3F_KEY_BUFFER_FORCE_LOWER (1 << 0)
#define T3F_KEY_BUFFER_FORCE_UPPER (1 << 1)

bool _t3f_initialize_keyboard(void);
void _t3f_uninitialize_keyboard(void);
void _t3f_handle_keyboard_event(ALLEGRO_EVENT * event);

void t3f_set_key_repeat(bool onoff);
bool t3f_key_held(int key);
bool t3f_key_pressed(int key);
bool t3f_use_key_press(int key);
bool t3f_key_released(int key);
bool t3f_use_key_release(int key);
void t3f_clear_key_states(void);

bool t3f_put_char(int c);
bool t3f_char_in_buffer(void);
int t3f_get_char(int flags);
void t3f_clear_chars(void);

#endif
