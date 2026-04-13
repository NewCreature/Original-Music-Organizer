#include "t3f.h"
#include "touch.h"

typedef struct
{

	int id; // track which id this touch is associated with

	/* user-facing members */
	bool active; // is this touch active?
	bool pressed;
	bool released;
	float real_x, real_y; // the actual screen coordinates
	float x, y; // coordinates transformed to for a view
	bool primary;

} _T3F_TOUCH_STATE;

/* touch data */
static _T3F_TOUCH_STATE * _t3f_touch_state = NULL;
static int _t3f_touch_slot = 2; // reserve slot 0 for mouse and 1 for primary

bool _t3f_initialize_touch_data(void)
{
  _t3f_touch_state = malloc(sizeof(_T3F_TOUCH_STATE) * T3F_MAX_TOUCHES);
  if(!_t3f_touch_state)
  {
    goto fail;
  }
  t3f_clear_touch_state();

	return true;

	fail:
	{
		_t3f_uninitialize_touch_data();
		return false;
	}
}

void _t3f_uninitialize_touch_data(void)
{
	if(_t3f_touch_state)
	{
		free(_t3f_touch_state);
		_t3f_touch_state = NULL;
	}
}

bool _t3f_initialize_touch(void)
{
	return al_install_touch_input();
}

void _t3f_uninitialize_touch(void)
{
  if(al_is_touch_input_installed())
  {
		al_uninstall_touch_input();
  }
}

static int _t3f_get_touch_slot_by_id(int id)
{
	int i;

	for(i = 2; i < T3F_MAX_TOUCHES; i++)
	{
		if(_t3f_touch_state[i].id == id)
		{
			return i;
		}
	}
	return -1;
}

void _t3f_handle_touch_event(ALLEGRO_EVENT * event)
{
  switch(event->type)
  {
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		{
			_t3f_touch_state[0].active = true;
			_t3f_touch_state[0].pressed = true;
			_t3f_touch_state[0].real_x = event->mouse.x;
			_t3f_touch_state[0].real_y = event->mouse.y;
			_t3f_touch_state[0].primary = true;
			break;
		}
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		{
			if(_t3f_touch_state[0].active)
			{
				_t3f_touch_state[0].active = false;
				_t3f_touch_state[0].real_x = event->mouse.x;
				_t3f_touch_state[0].real_y = event->mouse.y;
				_t3f_touch_state[0].released = true;
			}
			break;
		}
		case ALLEGRO_EVENT_MOUSE_AXES:
		{
			if(event->any.timestamp >= _t3f_get_mouse_warp_time())
			{
				if(_t3f_touch_state[0].active)
				{
					_t3f_touch_state[0].real_x = event->mouse.x;
					_t3f_touch_state[0].real_y = event->mouse.y;
				}
			}
			break;
		}
		case ALLEGRO_EVENT_MOUSE_WARPED:
		{
			if(event->any.timestamp >= _t3f_get_mouse_warp_time())
			{
				if(_t3f_touch_state[0].active)
				{
					_t3f_touch_state[0].real_x = event->mouse.x;
					_t3f_touch_state[0].real_y = event->mouse.y;
				}
			}
			break;
		}
		case ALLEGRO_EVENT_TOUCH_BEGIN:
		{
			_t3f_touch_state[_t3f_touch_slot].id = event->touch.id;
			_t3f_touch_state[_t3f_touch_slot].active = true;
			_t3f_touch_state[_t3f_touch_slot].real_x = event->touch.x;
			_t3f_touch_state[_t3f_touch_slot].real_y = event->touch.y;
			_t3f_touch_state[_t3f_touch_slot].primary = event->touch.primary;
			_t3f_touch_state[_t3f_touch_slot].pressed = true;
			_t3f_touch_slot++;
			if(_t3f_touch_slot >= T3F_MAX_TOUCHES)
			{
				_t3f_touch_slot = 2;
			}
			if(event->touch.primary)
			{
				_t3f_touch_state[0].id = event->touch.id;
				_t3f_touch_state[0].active = true;
				_t3f_touch_state[0].real_x = event->touch.x;
				_t3f_touch_state[0].real_y = event->touch.y;
				_t3f_touch_state[0].primary = event->touch.primary;
				_t3f_touch_state[0].pressed = true;
				_t3f_touch_state[1].id = event->touch.id;
				_t3f_touch_state[1].active = true;
				_t3f_touch_state[1].real_x = event->touch.x;
				_t3f_touch_state[1].real_y = event->touch.y;
				_t3f_touch_state[1].primary = event->touch.primary;
				_t3f_touch_state[1].pressed = true;
			}
			break;
		}

		case ALLEGRO_EVENT_TOUCH_MOVE:
		{
			int touch_slot = _t3f_get_touch_slot_by_id(event->touch.id);
			if(touch_slot >= 0 && _t3f_touch_state[touch_slot].active)
			{
				_t3f_touch_state[touch_slot].real_x = event->touch.x;
				_t3f_touch_state[touch_slot].real_y = event->touch.y;
				if(_t3f_touch_state[touch_slot].primary)
				{
					_t3f_touch_state[0].real_x = event->touch.x;
					_t3f_touch_state[0].real_y = event->touch.y;
					_t3f_touch_state[1].real_x = event->touch.x;
					_t3f_touch_state[1].real_y = event->touch.y;
				}
			}
			break;
		}

		case ALLEGRO_EVENT_TOUCH_END:
		case ALLEGRO_EVENT_TOUCH_CANCEL:
		{
			int touch_slot = _t3f_get_touch_slot_by_id(event->touch.id);
			if(touch_slot >= 0 && _t3f_touch_state[touch_slot].active)
			{
				_t3f_touch_state[touch_slot].active = false;
				_t3f_touch_state[touch_slot].real_x = event->touch.x;
				_t3f_touch_state[touch_slot].real_y = event->touch.y;
				_t3f_touch_state[touch_slot].released = true;
				_t3f_touch_state[touch_slot].id = -1; // invalidate slot
				if(_t3f_touch_state[touch_slot].primary)
				{
					_t3f_touch_state[0].active = false;
					_t3f_touch_state[0].real_x = event->touch.x;
					_t3f_touch_state[0].real_y = event->touch.y;
					_t3f_touch_state[0].released = true;
					_t3f_touch_state[0].id = -1; // invalidate slot
					_t3f_touch_state[1].active = false;
					_t3f_touch_state[1].real_x = event->touch.x;
					_t3f_touch_state[1].real_y = event->touch.y;
					_t3f_touch_state[1].released = true;
					_t3f_touch_state[1].id = -1; // invalidate slot
				}
			}
			break;
		}
  }
}

void _t3f_update_real_touch_pos(int point, int x, int y)
{
	_t3f_touch_state[point].real_x = x;
	_t3f_touch_state[point].real_y = y;
}

static void _t3f_update_single_touch_pos(int point, float translate_x, float scale_x, float translate_y, float scale_y)
{
  _t3f_touch_state[point].x = (_t3f_touch_state[point].real_x - translate_x) / scale_x;
  _t3f_touch_state[point].y = (_t3f_touch_state[point].real_y - translate_y) / scale_y;
}

void _t3f_update_touch_pos(int point, float translate_x, float scale_x, float translate_y, float scale_y)
{
  int i;

  if(point >= 0)
  {
    _t3f_update_single_touch_pos(point, translate_x, scale_x, translate_y, scale_y);
  }
  else
  {
    for(i = 1; i < T3F_MAX_TOUCHES; i++)
    {
      _t3f_update_single_touch_pos(i, translate_x, scale_x, translate_y, scale_y);
    }
  }
}

bool t3f_touch_active(int point)
{
  int i;

  if(point >= 0)
  {
    return _t3f_touch_state[point].active;
  }
  else
  {
    for(i = 2; i < T3F_MAX_TOUCHES; i++)
    {
      if(_t3f_touch_state[i].active)
      {
        return true;
      }
    }
  }
  return false;
}

bool t3f_touch_pressed(int point)
{
  int i;

  if(point >= 0)
  {
    return _t3f_touch_state[point].pressed;
  }
  else
  {
    for(i = 2; i < T3F_MAX_TOUCHES; i++)
    {
      if(_t3f_touch_state[i].pressed)
      {
        return true;
      }
    }
  }
  return false;
}

bool t3f_use_touch_press(int point)
{
  bool ret = t3f_touch_pressed(point);
  int i;

  if(point >= 0)
  {
    _t3f_touch_state[point].pressed = false;
  }
  else
  {
    for(i = 2; i < T3F_MAX_TOUCHES; i++)
    {
      if(_t3f_touch_state[i].pressed)
      {
        _t3f_touch_state[i].pressed = false;
        break;
      }
    }
  }
  return ret;
}

bool t3f_touch_released(int point)
{
  int i;

  if(point >= 0)
  {
    return _t3f_touch_state[point].released;
  }
  else
  {
    for(i = 2; i < T3F_MAX_TOUCHES; i++)
    {
      if(_t3f_touch_state[i].released)
      {
        return true;
      }
    }
  }
  return false;
}

bool t3f_use_touch_release(int point)
{
  bool ret = t3f_touch_released(point);
  int i;

  if(point >= 0)
  {
    _t3f_touch_state[point].released = false;
  }
  else
  {
    for(i = 2; i < T3F_MAX_TOUCHES; i++)
    {
      if(_t3f_touch_state[i].released)
      {
        _t3f_touch_state[i].released = false;
        break;
      }
    }
  }
  return ret;
}

float t3f_get_touch_x(int point)
{
	int i;

  if(point >= 0)
  {
    return _t3f_touch_state[point].x;
  }
  else
  {
    for(i = 2; i < T3F_MAX_TOUCHES; i++)
    {
      if(_t3f_touch_state[i].active)
      {
		return _t3f_touch_state[i].x;
      }
    }
  }
	return -100000;
}

float t3f_get_touch_y(int point)
{
	int i;

  if(point >= 0)
  {
    return _t3f_touch_state[point].y;
  }
  else
  {
    for(i = 2; i < T3F_MAX_TOUCHES; i++)
    {
      if(_t3f_touch_state[i].active)
      {
				return _t3f_touch_state[i].y;
      }
    }
  }
	return -100000;
}

void t3f_clear_touch_state(void)
{
	int i;

	memset(_t3f_touch_state, 0, sizeof(_T3F_TOUCH_STATE) * T3F_MAX_TOUCHES);
	for(i = 0; i < T3F_MAX_TOUCHES; i++)
	{
		_t3f_touch_state[i].id = -1;
	}
}
