#ifndef T3F_CONTROLLER_H
#define T3F_CONTROLLER_H

#ifdef __cplusplus
   extern "C" {
#endif

#include <allegro5/allegro5.h>

#define T3F_MAX_CONTROLLER_BINDINGS 32

#define T3F_CONTROLLER_BINDING_KEY               0
#define T3F_CONTROLLER_BINDING_JOYSTICK_BUTTON   1
#define T3F_CONTROLLER_BINDING_JOYSTICK_AXIS     2
#define T3F_CONTROLLER_BINDING_MOUSE_BUTTON      3
#define T3F_CONTROLLER_BINDING_MOUSE_AXIS        4

#define T3F_CONTROLLER_FLAG_AXIS_NEGATIVE        1
#define T3F_CONTROLLER_FLAG_AXIS_POSITIVE        2
#define T3F_CONTROLLER_FLAG_AXIS_ANALOGUE        4
#define T3F_CONTROLLER_FLAG_AXIS_INVERT          8
#define T3F_CONTROLLER_FLAG_AXIS_NO_ADJUST      16

#define T3F_CONTROLLER_AXIS_THRESHOLD         (0.3)

typedef struct
{

	int type;     // type of input (mouse, keyboard, joystick)
	int sub_type; // axis, button, etc.
	int button;   // which button (keyboard/joystick/mouse buttons)
	int joystick; // which joystick
	int stick;    // which stick (joystick)
	int axis;     // which axis
	float min, mid, max;
	int flags;

} T3F_CONTROLLER_BINDING;

typedef struct
{

	int flags;
	float pos;
	float delta;

	bool was_held;
	bool down;
	bool held;
	bool pressed;
	bool released;

} T3F_CONTROLLER_STATE;

typedef struct
{

	T3F_CONTROLLER_BINDING binding[T3F_MAX_CONTROLLER_BINDINGS];
	T3F_CONTROLLER_STATE state[T3F_MAX_CONTROLLER_BINDINGS];
	int bindings;
  const char * device_name; // remap all bindings to controller that matches this

} T3F_CONTROLLER;

T3F_CONTROLLER * t3f_create_controller(int bindings);
void t3f_destroy_controller(T3F_CONTROLLER * cp);
bool t3f_bind_controller(T3F_CONTROLLER * cp, int binding);
const char * t3f_get_controller_name(T3F_CONTROLLER * cp, int binding);
const char * t3f_get_controller_binding_name(T3F_CONTROLLER * cp, int binding);
void t3f_write_controller_config(ALLEGRO_CONFIG * acp, const char * section, T3F_CONTROLLER * cp);
bool t3f_read_controller_config(ALLEGRO_CONFIG * acp, const char * section, T3F_CONTROLLER * cp);
void t3f_read_controller(T3F_CONTROLLER * cp);
void t3f_update_controller(T3F_CONTROLLER * cp);
void t3f_clear_controller_state(T3F_CONTROLLER * cp);

#ifdef __cplusplus
	}
#endif

#endif
