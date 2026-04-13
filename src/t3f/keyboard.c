#include <ctype.h>
#include "t3f.h"
#include "keyboard.h"

#define _T3F_KEY_BUFFER_MAX 256

typedef struct
{

	bool held;
	bool pressed;
	bool released;

} _T3F_KEY_STATE;

typedef struct
{

  int data[_T3F_KEY_BUFFER_MAX];
  int count;

} _T3F_KEY_BUFFER;

/* keyboard data */
static _T3F_KEY_STATE * _t3f_key_state = NULL;
static _T3F_KEY_BUFFER * _t3f_key_buffer = NULL;
static bool _t3f_key_repeat_enabled = true;

void t3f_set_key_repeat(bool onoff)
{
	_t3f_key_repeat_enabled = onoff;
}

static void _handle_key_press(int keycode)
{
	_t3f_key_state[keycode].held = true;
	_t3f_key_state[keycode].pressed = true;
	_t3f_key_state[0].held = true;
	_t3f_key_state[0].pressed = true;
	_t3f_key_state[0].released = false;
}

static void _handle_key_release(int keycode)
{
	if(_t3f_key_state[keycode].held)
	{
		_t3f_key_state[keycode].held = false;
		_t3f_key_state[keycode].released = true;
		_t3f_key_state[0].held = false;
		_t3f_key_state[0].released = true;
		_t3f_key_state[0].pressed = false;
	}
}

void _t3f_handle_keyboard_event(ALLEGRO_EVENT * event)
{
  switch(event->type)
  {
		/* key was pressed or repeated */
		case ALLEGRO_EVENT_KEY_DOWN:
		{
			_handle_key_press(event->keyboard.keycode);
			#ifdef ALLEGRO_MACOSX
				if(event->keyboard.keycode == ALLEGRO_KEY_LSHIFT)
				{
					_handle_key_press(ALLEGRO_KEY_RSHIFT);
				}
			#endif
			break;
		}

		/* key was released */
		case ALLEGRO_EVENT_KEY_UP:
		{
			_handle_key_release(event->keyboard.keycode);
			#ifdef ALLEGRO_MACOSX
				if(event->keyboard.keycode == ALLEGRO_KEY_LSHIFT)
				{
					_handle_key_release(ALLEGRO_KEY_RSHIFT);
				}
			#endif
			break;
		}

		/* a character was entered */
		case ALLEGRO_EVENT_KEY_CHAR:
		{
			if(_t3f_key_repeat_enabled || !event->keyboard.repeat)
			{
				if(event->keyboard.unichar != -1)
				{
					t3f_put_char(event->keyboard.unichar);
				}
			}
			if(event->keyboard.repeat && _t3f_key_repeat_enabled)
			{
				_handle_key_press(event->keyboard.keycode);
			}
			break;
		}
  }
}

bool _t3f_initialize_keyboard(void)
{
  if(!al_install_keyboard())
  {
    goto fail;
  }
  _t3f_key_state = malloc(sizeof(_T3F_KEY_STATE) * ALLEGRO_KEY_MAX);
  if(!_t3f_key_state)
  {
    goto fail;
  }
  memset(_t3f_key_state, 0, sizeof(_T3F_KEY_STATE) * ALLEGRO_KEY_MAX);
  _t3f_key_buffer = malloc(sizeof(_T3F_KEY_BUFFER));
  if(!_t3f_key_buffer)
  {
    goto fail;
  }
  memset(_t3f_key_buffer, 0, sizeof(_T3F_KEY_BUFFER));

  return true;

  fail:
  {
    _t3f_uninitialize_keyboard();

    return false;
  }
}

void _t3f_uninitialize_keyboard(void)
{
  if(al_is_keyboard_installed())
  {
    if(_t3f_key_buffer)
    {
      free(_t3f_key_buffer);
      _t3f_key_buffer = NULL;
    }
    if(_t3f_key_state)
    {
      free(_t3f_key_state);
      _t3f_key_state = NULL;
    }
    al_uninstall_keyboard();
  }
}

bool t3f_key_held(int key)
{
  return _t3f_key_state[key].held;
}

bool t3f_key_pressed(int key)
{
  return _t3f_key_state[key].pressed;
}

bool t3f_use_key_press(int key)
{
	bool ret = _t3f_key_state[key].pressed;

	_t3f_key_state[key].pressed = false;

  return ret;
}

bool t3f_key_released(int key)
{
  return _t3f_key_state[key].released;
}

bool t3f_use_key_release(int key)
{
	bool ret = _t3f_key_state[key].released;

  _t3f_key_state[key].released = false;

  return ret;
}

void t3f_clear_key_states(void)
{
	memset(_t3f_key_state, 0, sizeof(_T3F_KEY_STATE) * ALLEGRO_KEY_MAX);
}

bool t3f_put_char(int c)
{
	if(_t3f_key_buffer->count < _T3F_KEY_BUFFER_MAX)
	{
		_t3f_key_buffer->data[_t3f_key_buffer->count] = c;
		_t3f_key_buffer->count++;
		return true;
	}
	return false;
}

bool t3f_char_in_buffer(void)
{
	return _t3f_key_buffer->count > 0;
}

int t3f_get_char(int flags)
{
	int rkey = 0;

	if(_t3f_key_buffer->count > 0)
	{
		_t3f_key_buffer->count--;
		rkey = _t3f_key_buffer->data[_t3f_key_buffer->count];
		if(flags & T3F_KEY_BUFFER_FORCE_LOWER)
		{
      rkey = tolower(rkey);
		}
		else if(flags & T3F_KEY_BUFFER_FORCE_UPPER)
		{
      rkey = toupper(rkey);
		}
	}
	return rkey;
}

void t3f_clear_chars(void)
{
	_t3f_key_buffer->count = 0;
}
