#include "t3f.h"
#include "mouse.h"
#include "touch.h"

#define T3F_MAX_MOUSE_BUTTONS 16

typedef struct
{

	bool held;
	bool pressed;
	bool released;

} _T3F_MOUSE_BUTTON_STATE;

typedef struct
{

	/* Allegro events give us the 'real' mouse coordinates */
	int real_x;
	int real_y;

	/* we'll use the current view transformation to get our 'virtual' mouse coordinates */
	float x;
	float y;
	int z;

	/* mouse position deltas (sorta like mickeys) */
	float dx;
	float dy;
	int dz;

	_T3F_MOUSE_BUTTON_STATE button[T3F_MAX_MOUSE_BUTTONS];

	bool visible;
	double warp_time;

} _T3F_MOUSE_STATE;

static _T3F_MOUSE_STATE * _t3f_mouse_state = NULL;

bool _t3f_initialize_mouse(void)
{
  if(!al_install_mouse())
  {
    goto fail;
  }
  _t3f_mouse_state = malloc(sizeof(_T3F_MOUSE_STATE));
  if(!_t3f_mouse_state)
  {
    goto fail;
  }
  memset(_t3f_mouse_state, 0, sizeof(_T3F_MOUSE_STATE));

  return true;

  fail:
  {
    _t3f_uninitialize_mouse();

    return false;
  }
}

void _t3f_uninitialize_mouse(void)
{
  if(_t3f_mouse_state)
  {
    free(_t3f_mouse_state);
    _t3f_mouse_state = NULL;
  }
  if(al_is_mouse_installed())
  {
    al_uninstall_mouse();
  }
}

void _t3f_handle_mouse_event(ALLEGRO_EVENT * event)
{
  switch(event->type)
  {
 		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		{
			_t3f_mouse_state->button[event->mouse.button - 1].held = true;
			_t3f_mouse_state->button[event->mouse.button - 1].pressed = true;
			_t3f_mouse_state->real_x = event->mouse.x;
			_t3f_mouse_state->real_y = event->mouse.y;
			_t3f_mouse_state->z = event->mouse.z;
			break;
		}
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		{
			if(_t3f_mouse_state->button[event->mouse.button - 1].held)
			{
				_t3f_mouse_state->button[event->mouse.button - 1].held = false;
				_t3f_mouse_state->button[event->mouse.button - 1].released = true;
				_t3f_mouse_state->real_x = event->mouse.x;
				_t3f_mouse_state->real_y = event->mouse.y;
				_t3f_mouse_state->z = event->mouse.z;
			}
			break;
		}
		case ALLEGRO_EVENT_MOUSE_AXES:
		{
			if(event->any.timestamp >= _t3f_get_mouse_warp_time())
			{
				_t3f_mouse_state->real_x = event->mouse.x;
				_t3f_mouse_state->real_y = event->mouse.y;
				_t3f_mouse_state->z = event->mouse.z;
			}
			_t3f_mouse_state->dx += event->mouse.dx;
			_t3f_mouse_state->dy += event->mouse.dy;
			break;
		}
		case ALLEGRO_EVENT_MOUSE_WARPED:
		{
			if(event->any.timestamp >= _t3f_get_mouse_warp_time())
			{
				_t3f_mouse_state->real_x = event->mouse.x;
				_t3f_mouse_state->real_y = event->mouse.y;
			}
			break;
		}
		case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
		{
			_t3f_mouse_state->visible = false;
			break;
		}
		case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
		{
			_t3f_mouse_state->visible = true;
			break;
		}
  }
}

void _t3f_update_mouse_pos(float translate_x, float scale_x, float translate_y, float scale_y)
{
	_t3f_mouse_state->x = (_t3f_mouse_state->real_x - translate_x) / scale_x;
	_t3f_mouse_state->y = (_t3f_mouse_state->real_y - translate_y) / scale_y;
}

double _t3f_get_mouse_warp_time(void)
{
	return _t3f_mouse_state->warp_time;
}

void _t3f_update_mouse_warp_time(double when)
{
	_t3f_mouse_state->warp_time = when;
}

/* API */
bool t3f_mouse_button_held(int button)
{
	return _t3f_mouse_state->button[button].held;
}

bool t3f_mouse_button_pressed(int button)
{
  return _t3f_mouse_state->button[button].pressed;
}

bool t3f_use_mouse_button_press(int button)
{
	bool ret = _t3f_mouse_state->button[button].pressed;

  _t3f_mouse_state->button[button].pressed = false;

  return ret;
}

bool t3f_mouse_button_released(int button)
{
  return _t3f_mouse_state->button[button].released;
}

bool t3f_use_mouse_button_release(int button)
{
	bool ret = _t3f_mouse_state->button[button].released;

  _t3f_mouse_state->button[button].released = false;

  return ret;
}

float t3f_get_mouse_x(void)
{
  return _t3f_mouse_state->x;
}

float t3f_get_mouse_y(void)
{
  return _t3f_mouse_state->y;
}

int t3f_get_mouse_z(void)
{
	return _t3f_mouse_state->z;
}

bool t3f_get_mouse_mickeys(int * x, int * y, int * z)
{
	if(x)
	{
		*x = _t3f_mouse_state->dx;
		_t3f_mouse_state->dx = 0;
	}
	if(y)
	{
		*y = _t3f_mouse_state->dy;
		_t3f_mouse_state->dy = 0;
	}
	if(z)
	{
		*z = _t3f_mouse_state->dz;
		_t3f_mouse_state->dz = 0;
	}
	return true;
}

static void _t3f_update_mouse_xy(float x, float y)
{
	_t3f_mouse_state->real_x = x;
	_t3f_mouse_state->real_y = y;
	_t3f_update_real_touch_pos(0, x, y);
	t3f_select_input_view(t3f_current_view);
}

/* set the mouse coordinate to (x, y) in the currently active view */
void t3f_set_mouse_xy(float x, float y)
{
	al_transform_coordinates(&t3f_current_view->transform, &x, &y);
	al_set_mouse_xy(t3f_display, x, y);
	_t3f_update_mouse_xy(x, y);
	_t3f_update_mouse_warp_time(al_get_time());
}

void t3f_clear_mouse_state(void)
{
	memset(_t3f_mouse_state, 0, sizeof(_T3F_MOUSE_STATE));
}
