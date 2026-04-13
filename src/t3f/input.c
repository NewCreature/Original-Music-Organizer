#include "t3f.h"
#include "input.h"
#include "mouse.h"
#include "keyboard.h"

static bool _t3f_input_initialized = false;
ALLEGRO_JOYSTICK_STATE _input_state_fudging_helper[T3F_MAX_JOYSTICKS];

static void _reset_input_state_fudging(void)
{
  int i;

  #ifdef ALLEGRO_MACOSX
    for(i = 0; i < T3F_MAX_JOYSTICKS; i++)
    {
      _input_state_fudging_helper[i].stick[2].axis[0] = 100.0;
      _input_state_fudging_helper[i].stick[3].axis[0] = 100.0;
    }
  #else
    #ifdef ALLEGRO_UNIX
      for(i = 0; i < T3F_MAX_JOYSTICKS; i++)
      {
        _input_state_fudging_helper[i].stick[1].axis[0] = 100.0;
        _input_state_fudging_helper[i].stick[2].axis[1] = 100.0;
      }
    #endif
  #endif
}

bool t3f_initialize_input(int flags)
{
  if(!_t3f_input_initialized)
  {
    _reset_input_state_fudging();
    _t3f_input_initialized = true;
  }
  return true;
}

void t3f_deinitialize_input(void)
{
  if(_t3f_input_initialized)
  {
    _t3f_input_initialized = false;
  }
}

T3F_INPUT_HANDLER * t3f_create_input_handler(int type)
{
  T3F_INPUT_HANDLER * input_handler = NULL;

  if(!_t3f_input_initialized)
  {
    return NULL;
  }

  input_handler = malloc(sizeof(T3F_INPUT_HANDLER));
  if(!input_handler)
  {
    goto fail;
  }
  memset(input_handler, 0, sizeof(T3F_INPUT_HANDLER));

  input_handler->type = type;
  switch(input_handler->type)
  {
    case T3F_INPUT_HANDLER_TYPE_GAMEPAD:
    {
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Left Analog X
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Left Analog Y
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Right Analog X
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Right Analog Y
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // D-Pad Left
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // D-Pad Right
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // D-Pad Up
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // D-Pad Down
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Left Trigger
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Right Trigger
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // A
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // B
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // X
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Y
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // L
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // R
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // L3
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // R3
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Select
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Start
      break;
    }
    case T3F_INPUT_HANDLER_TYPE_MOUSE:
    {
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_ABSOLUTE_AXIS); // X-Axis
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_ABSOLUTE_AXIS); // Y-Axis
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_ABSOLUTE_AXIS); // Z-Axis
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_ABSOLUTE_AXIS); // W-Axis
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Button
      break;
    }
  }
  return input_handler;

  fail:
  {
    if(input_handler)
    {
      free(input_handler);
    }
    return NULL;
  }
}

void t3f_destroy_input_handler(T3F_INPUT_HANDLER * input_handler)
{
  free(input_handler);
}

bool t3f_add_input_handler_element(T3F_INPUT_HANDLER * input_handler, int type)
{
  T3F_INPUT_HANDLER_ELEMENT * element = NULL;
  int i;

  element = malloc(sizeof(T3F_INPUT_HANDLER_ELEMENT) * (input_handler->elements + 1));
  if(!element)
  {
    goto fail;
  }
  for(i = 0; i < input_handler->elements; i++)
  {
    memcpy(&element[i], &input_handler->element[i], sizeof(T3F_INPUT_HANDLER_ELEMENT));
  }
  memset(&element[i], 0, sizeof(T3F_INPUT_HANDLER_ELEMENT));
  element[i].type = type;
  free(input_handler->element);
  input_handler->element = element;
  input_handler->elements++;

  return true;

  fail:
  {
    if(element)
    {
      free(element);
    }
    return false;
  }
}

void t3f_bind_input_handler_element(T3F_INPUT_HANDLER * input_handler, int element, int slot, int device_type, int device_number, int device_element)
{
  input_handler->element[element].binding[slot].device_type = device_type;
  input_handler->element[element].binding[slot].device_number = device_number;
  input_handler->element[element].binding[slot].device_element = device_element;
}

bool t3f_map_input_for_xbox_controller(T3F_INPUT_HANDLER * input_handler, int joystick)
{
  if(input_handler->type != T3F_INPUT_HANDLER_TYPE_GAMEPAD)
  {
    return false;
  }

  /* Left Analog X */
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].dead_zone = T3F_INPUT_DEFAULT_STICK_DEAD_ZONE;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].threshold = T3F_INPUT_DEFAULT_STICK_THRESHOLD;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].stick_elements = 0;

  /* Left Analog Y */
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].dead_zone = T3F_INPUT_DEFAULT_STICK_DEAD_ZONE;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].threshold = T3F_INPUT_DEFAULT_STICK_THRESHOLD;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].stick_elements = 0;

  /* Right Analog X */
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].dead_zone = T3F_INPUT_DEFAULT_STICK_DEAD_ZONE;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].threshold = T3F_INPUT_DEFAULT_STICK_THRESHOLD;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].stick_elements = 0;

  /* Right Analog Y */
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].dead_zone = T3F_INPUT_DEFAULT_STICK_DEAD_ZONE;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].threshold = T3F_INPUT_DEFAULT_STICK_THRESHOLD;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].stick_elements = 0;

  /* D-Pad Left */
  input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].threshold = 0.1;
  input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].stick_elements = 0;

  /* D-Pad Right */
  input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].threshold = 0.1;
  input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].stick_elements = 0;

  /* D-Pad Up */
  input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].threshold = 0.1;
  input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].stick_elements = 0;

  /* D-Pad Down */
  input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].threshold = 0.1;
  input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].stick_elements = 0;

  /* Left Trigger */
  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].threshold = 0.5;
//  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].scale = 0.5;
//  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].offset = 1.0;
  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].stick_elements = 0;

  /* Right Trigger */
  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].threshold = 0.5;
//  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].scale = 0.5;
//  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].offset = 1.0;
  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].stick_elements = 0;

  /* A */
  input_handler->element[T3F_GAMEPAD_A].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_A].binding[0].stick_elements = 0;

  /* B */
  input_handler->element[T3F_GAMEPAD_B].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_B].binding[0].stick_elements = 0;

  /* X */
  input_handler->element[T3F_GAMEPAD_X].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_X].binding[0].stick_elements = 0;

  /* Y */
  input_handler->element[T3F_GAMEPAD_Y].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_Y].binding[0].stick_elements = 0;

  /* L */
  input_handler->element[T3F_GAMEPAD_L].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_L].binding[0].stick_elements = 0;

  /* R */
  input_handler->element[T3F_GAMEPAD_R].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_R].binding[0].stick_elements = 0;

  /* L3 */
  input_handler->element[T3F_GAMEPAD_L3].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_L3].binding[0].stick_elements = 0;

  /* R3 */
  input_handler->element[T3F_GAMEPAD_R3].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_R3].binding[0].stick_elements = 0;

  /* Start */
  input_handler->element[T3F_GAMEPAD_START].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_START].binding[0].stick_elements = 0;

  /* Select */
  input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_SELECT].binding[0].stick_elements = 0;

  #ifdef ALLEGRO_MACOSX

    /* Left Analog X */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].device_element = 0;

    /* Left Analog Y */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].device_element = 1;

    /* Right Analog X */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].device_element = 2;

    /* Right Analog Y */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].device_element = 3;

    /* D-Pad Left */
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_element = 6;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_element_dir = -1.0;

    /* D-Pad Right */
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_element = 6;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_element_dir = 1.0;

    /* D-Pad Up */
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_element = 7;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_element_dir = -1.0;

    /* D-Pad Down */
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_element = 7;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_element_dir = 1.0;

    /* Left Trigger */
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].device_element = 4;

    /* Right Trigger */
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].device_element = 5;

    /* A */
    input_handler->element[T3F_GAMEPAD_A].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_A].binding[0].device_element = 8;

    /* B */
    input_handler->element[T3F_GAMEPAD_B].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_B].binding[0].device_element = 9;

    /* X */
    input_handler->element[T3F_GAMEPAD_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_X].binding[0].device_element = 10;

    /* Y */
    input_handler->element[T3F_GAMEPAD_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_Y].binding[0].device_element = 11;

    /* L */
    input_handler->element[T3F_GAMEPAD_L].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L].binding[0].device_element = 12;

    /* R */
    input_handler->element[T3F_GAMEPAD_R].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R].binding[0].device_element = 13;

    /* L3 */
    input_handler->element[T3F_GAMEPAD_L3].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L3].binding[0].device_element = 16;

    /* R3 */
    input_handler->element[T3F_GAMEPAD_R3].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R3].binding[0].device_element = 17;

    /* Start */
    input_handler->element[T3F_GAMEPAD_START].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_START].binding[0].device_element = 15;

    /* Select */
    input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_element = 14;

    return true;

  #endif

  #ifdef ALLEGRO_WINDOWS

    /* Left Analog X */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].device_element = 2;

    /* Left Analog Y */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].device_element = 3;

    /* Right Analog X */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].device_element = 4;

    /* Right Analog Y */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].device_element = 5;

    /* D-Pad Left */
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_element = 0;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_element_dir = -1.0;

    /* D-Pad Right */
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_element = 0;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_element_dir = 1.0;

    /* D-Pad Up */
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_element = 1;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_element_dir = -1.0;

    /* D-Pad Down */
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_element = 1;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_element_dir = 1.0;

    /* Left Trigger */
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].device_element = 6;

    /* Right Trigger */
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].device_element = 7;

    /* A */
    input_handler->element[T3F_GAMEPAD_A].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_A].binding[0].device_element = 8;

    /* B */
    input_handler->element[T3F_GAMEPAD_B].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_B].binding[0].device_element = 9;

    /* X */
    input_handler->element[T3F_GAMEPAD_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_X].binding[0].device_element = 10;

    /* Y */
    input_handler->element[T3F_GAMEPAD_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_Y].binding[0].device_element = 11;

    /* L */
    input_handler->element[T3F_GAMEPAD_L].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L].binding[0].device_element = 12;

    /* R */
    input_handler->element[T3F_GAMEPAD_R].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R].binding[0].device_element = 13;

    /* L3 */
    input_handler->element[T3F_GAMEPAD_L3].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L3].binding[0].device_element = 17;

    /* R3 */
    input_handler->element[T3F_GAMEPAD_R3].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R3].binding[0].device_element = 18;

    /* Select */
    input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_element = 14;

    /* Start */
    input_handler->element[T3F_GAMEPAD_START].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_START].binding[0].device_element = 15;

    return true;

  #endif

  #ifdef ALLEGRO_UNIX

    /* Left Analog X */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].device_element = 0;

    /* Left Analog Y */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].device_element = 1;

    /* Right Analog X */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].device_element = 3;

    /* Right Analog Y */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].device_element = 4;

    /* D-Pad Left */
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_element = 6;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_element_dir = -1.0;

    /* D-Pad Right */
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_element = 6;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_element_dir = 1.0;

    /* D-Pad Up */
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_element = 7;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_element_dir = -1.0;

    /* D-Pad Down */
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_element = 7;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_element_dir = 1.0;

    /* Left Trigger */
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].device_element = 2;

    /* Right Trigger */
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].device_element = 5;

    /* A */
    input_handler->element[T3F_GAMEPAD_A].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_A].binding[0].device_element = 8;

    /* B */
    input_handler->element[T3F_GAMEPAD_B].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_B].binding[0].device_element = 9;

    /* X */
    input_handler->element[T3F_GAMEPAD_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_X].binding[0].device_element = 10;

    /* Y */
    input_handler->element[T3F_GAMEPAD_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_Y].binding[0].device_element = 11;

    /* L */
    input_handler->element[T3F_GAMEPAD_L].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L].binding[0].device_element = 12;

    /* R */
    input_handler->element[T3F_GAMEPAD_R].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R].binding[0].device_element = 13;

    /* wired version */
    if(al_get_joystick_num_buttons(t3f_joystick[joystick]) > 10)
    {
      /* L3 */
      input_handler->element[T3F_GAMEPAD_L3].binding[0].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_L3].binding[0].device_element = 17;

      /* R3 */
      input_handler->element[T3F_GAMEPAD_R3].binding[0].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_R3].binding[0].device_element = 18;

      /* Start */
      input_handler->element[T3F_GAMEPAD_START].binding[0].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_START].binding[0].device_element = 15;

      /* Select */
      input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_element = 14;
    }

    /* wireless version */
    else
    {
      /* L3 */
      input_handler->element[T3F_GAMEPAD_L3].binding[0].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_L3].binding[0].device_element = 16;

      /* R3 */
      input_handler->element[T3F_GAMEPAD_R3].binding[0].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_R3].binding[0].device_element = 17;

      /* Start */
      input_handler->element[T3F_GAMEPAD_START].binding[0].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_START].binding[0].device_element = 15;

      /* Select */
      input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_element = 14;
    }

    return true;

  #endif

  #ifdef ALLEGRO_ANDROID

    /* Left Analog X */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].device_element = 0;

    /* Left Analog Y */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].device_element = 1;

    /* Right Analog X */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].device_element = 2;

    /* Right Analog Y */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].device_element = 3;

    /* D-Pad Left */
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_element = 12;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].binding[0].device_element_dir = -1.0;

    /* D-Pad Right */
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_element = 13;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].binding[0].device_element_dir = 1.0;

    /* D-Pad Up */
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_element = 14;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].binding[0].device_element_dir = -1.0;

    /* D-Pad Down */
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_element = 15;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].binding[0].device_element_dir = 1.0;

    /* Left Trigger */
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].binding[0].device_element = 4;

    /* Right Trigger */
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].binding[0].device_element = 5;

    /* A */
    input_handler->element[T3F_GAMEPAD_A].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_A].binding[0].device_element = 6;

    /* B */
    input_handler->element[T3F_GAMEPAD_B].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_B].binding[0].device_element = 7;

    /* X */
    input_handler->element[T3F_GAMEPAD_X].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_X].binding[0].device_element = 8;

    /* Y */
    input_handler->element[T3F_GAMEPAD_Y].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_Y].binding[0].device_element = 9;

    /* L */
    input_handler->element[T3F_GAMEPAD_L].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L].binding[0].device_element = 10;

    /* R */
    input_handler->element[T3F_GAMEPAD_R].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R].binding[0].device_element = 11;

    /* L3 */
    input_handler->element[T3F_GAMEPAD_L3].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L3].binding[0].device_element = 19;

    /* R3 */
    input_handler->element[T3F_GAMEPAD_R3].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R3].binding[0].device_element = 20;

    /* Start */
    input_handler->element[T3F_GAMEPAD_START].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_START].binding[0].device_element = 16;

    /* Select */
    input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_SELECT].binding[0].device_element = 17;

    return true;

  #endif

}

bool t3f_map_input_for_mouse(T3F_INPUT_HANDLER * input_handler)
{
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_X, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 0);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_Y, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 1);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_Z, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 2);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_W, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 3);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_1, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 4);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_2, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 5);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_3, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 6);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_4, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 7);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_5, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 8);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_6, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 9);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_7, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 10);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_8, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 11);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_9, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 12);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_10, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 13);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_11, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 14);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_12, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 15);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_13, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 16);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_14, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 17);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_15, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 18);
  t3f_bind_input_handler_element(input_handler, T3F_MOUSE_BUTTON_16, 0, T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE, 0, 19);
  return true;
}

static void update_input_device(int device)
{
  #ifdef ALLEGRO_MACOSX
    if(t3f_joystick_state_updated[device])
    {
      /* triggers */
      if(_input_state_fudging_helper[device].stick[2].axis[0] < 100.0)
      {
        t3f_joystick_state[device].stick[2].axis[0] = (t3f_joystick_state[device].stick[2].axis[0] + 1.0) / 2.0;
      }
      if(_input_state_fudging_helper[device].stick[3].axis[0] < 100.0)
      {
        t3f_joystick_state[device].stick[3].axis[0] = (t3f_joystick_state[device].stick[3].axis[0] + 1.0) / 2.0;
      }

      t3f_joystick_state_updated[device] = false;
    }
    return;
  #endif

  #ifdef ALLEGRO_UNIX
    if(t3f_joystick_state_updated[device])
    {
      /* triggers */
      if(_input_state_fudging_helper[device].stick[1].axis[0] < 100.0)
      {
        t3f_joystick_state[device].stick[1].axis[0] = (t3f_joystick_state[device].stick[1].axis[0] + 1.0) / 2.0;
      }
      if(_input_state_fudging_helper[device].stick[2].axis[1] < 100.0)
      {
        t3f_joystick_state[device].stick[2].axis[1] = (t3f_joystick_state[device].stick[2].axis[1] + 1.0) / 2.0;
      }

      t3f_joystick_state_updated[device] = false;
    }
    return;
  #endif
}

static void update_input_handler_element_state_keyboard(T3F_INPUT_HANDLER_ELEMENT * element, int slot)
{
  if(t3f_key_held(element->binding[slot].device_element))
  {
    element->binding_held[slot] = true;
  }
  else
  {
    element->binding_held[slot] = false;
  }
}

static void update_input_handler_element_state_mouse(T3F_INPUT_HANDLER_ELEMENT * element, int slot)
{
  int button;

  switch(element->binding[slot].device_element)
  {
    case T3F_MOUSE_X:
    {
      element->val = t3f_get_mouse_x();
      break;
    }
    case T3F_MOUSE_Y:
    {
      element->val = t3f_get_mouse_y();
      break;
    }
    case T3F_MOUSE_Z:
    {
      element->val = t3f_get_mouse_z();
      break;
    }
    case T3F_MOUSE_W: // placeholder for mouse_w
    {
      element->val = 0;
    }
    default:
    {
      button = element->binding[slot].device_element - T3F_MOUSE_BUTTON_1;
      if(button >= 0)
      {
        if(t3f_mouse_button_held(button))
        {
          element->binding_held[slot] = true;
        }
        else
        {
          element->binding_held[slot] = false;
        }
      }
      break;
    }
  }
}

static void update_input_handler_element_joystick_cache(T3F_INPUT_HANDLER_ELEMENT * element, int slot)
{
  int i, j;

  if(element->binding[slot].stick_elements <= 0)
  {
    element->binding[slot].stick_elements = 0;
    for(i = 0; i < al_get_joystick_num_sticks(t3f_joystick[element->binding[slot].device_number]); i++)
    {
      for(j = 0; j < al_get_joystick_num_axes(t3f_joystick[element->binding[slot].device_number], i); j++)
      {
        element->binding[slot].stick[element->binding[slot].stick_elements] = i;
        element->binding[slot].axis[element->binding[slot].stick_elements] = j;
        element->binding[slot].stick_elements++;
      }
    }
  }
}

static void update_input_handler_element_state_joystick(T3F_INPUT_HANDLER_ELEMENT * element, int slot)
{
  float tval;
  float val;

  update_input_handler_element_joystick_cache(element, slot);
  update_input_device(element->binding[slot].device_number);
  if(element->binding[slot].device_element < element->binding[slot].stick_elements)
  {
    val = (t3f_joystick_state[element->binding[slot].device_number].stick[element->binding[slot].stick[element->binding[slot].device_element]].axis[element->binding[slot].axis[element->binding[slot].device_element]] + element->binding[slot].offset) * (element->binding[slot].scale > 0.0 ? element->binding[slot].scale : 1.0);
    if(fabs(val) >= element->binding[slot].dead_zone)
    {
      element->val = val;
      if(element->val < 0.0)
      {
        element->val = (element->val + element->binding[slot].dead_zone) / (1.0 - element->binding[slot].dead_zone);
      }
      else
      {
        element->val = (element->val - element->binding[slot].dead_zone) / (1.0 - element->binding[slot].dead_zone);
      }
    }
    else
    {
      element->val = 0.0;
    }
    if(element->binding[slot].device_element_dir < 0.0)
    {
      tval = -element->val;
    }
    else if(element->binding[slot].device_element_dir > 0.0)
    {
      tval = element->val;
    }
    else
    {
      tval = fabs(element->val);
    }
    if(tval >= element->binding[slot].threshold)
    {
      element->binding_held[slot] = true;
    }
    else
    {
      element->binding_held[slot] = false;
    }
  }
  else
  {
    if(t3f_joystick_state[element->binding[slot].device_number].button[element->binding[slot].device_element - element->binding[slot].stick_elements])
    {
      element->binding_held[slot] = true;
    }
    else
    {
      element->binding_held[slot] = false;
    }
  }
}

static void update_input_handler_element_state(T3F_INPUT_HANDLER_ELEMENT * element)
{
  int i;
  bool held = false;

  for(i = 0; i < T3F_INPUT_HANDLER_MAX_BINDINGS; i++)
  {
    if(element->binding[i].device_type != T3F_INPUT_HANDLER_DEVICE_TYPE_NONE)
    {
      switch(element->binding[i].device_type)
      {
        case T3F_INPUT_HANDLER_DEVICE_TYPE_KEYBOARD:
        {
          update_input_handler_element_state_keyboard(element, i);
          break;
        }
        case T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE:
        {
          update_input_handler_element_state_mouse(element, i);
          break;
        }
        case T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK:
        {
          update_input_handler_element_state_joystick(element, i);
          break;
        }
      }
    }
    if(element->binding_held[i])
    {
      held = true;
    }
  }
  
  /* if any of the bindings for this inpur are held, handle press/release */
  if(held)
  {
    if(!element->held)
    {
      element->pressed = true;
    }
    else
    {
      element->pressed = false;
    }
    element->held = true;
    element->released = false;
  }
  else
  {
    if(element->held)
    {
      element->released = true;
    }
    else
    {
      element->released = false;
    }
    element->held = false;
    element->pressed = false;
  }
}

void t3f_update_input_handler_state(T3F_INPUT_HANDLER * input_handler)
{
  int i;

  for(i = 0; i < input_handler->elements; i++)
  {
    update_input_handler_element_state(&input_handler->element[i]);
  }
}

void t3f_clear_input_handler_state(T3F_INPUT_HANDLER * input_handler)
{
  int i;

  for(i = 0; i < input_handler->elements; i++)
  {
    input_handler->element[i].held = false;
    input_handler->element[i].pressed = false;
    input_handler->element[i].released = false;
  }
}

bool t3f_input_held(T3F_INPUT_HANDLER * input_handler, int element)
{
  return input_handler->element[element].held;
}

bool t3f_input_pressed(T3F_INPUT_HANDLER * input_handler, int element)
{
  return input_handler->element[element].pressed;
}

bool t3f_use_input_press(T3F_INPUT_HANDLER * input_handler, int element)
{
  bool ret = input_handler->element[element].pressed;

  input_handler->element[element].pressed = false;

  return ret;
}

bool t3f_input_released(T3F_INPUT_HANDLER * input_handler, int element)
{
  return input_handler->element[element].released;
}

bool t3f_use_input_release(T3F_INPUT_HANDLER * input_handler, int element)
{
  bool ret = input_handler->element[element].released;

  input_handler->element[element].released = false;

  return ret;
}

float t3f_get_input_val(T3F_INPUT_HANDLER * input_handler, int element)
{
  return input_handler->element[element].val;
}

void _t3f_input_handle_joystick_event(ALLEGRO_EVENT * event)
{
  int joy_num = -1;

  if(event->type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION)
  {
    _reset_input_state_fudging();
  }

  #ifdef ALLEGRO_MACOSX
    if(event->type == ALLEGRO_EVENT_JOYSTICK_AXIS)
    {
      if(event->joystick.stick == 2)
      {
        joy_num = t3f_get_joystick_number(event->joystick.id);
        if(joy_num >= 0)
        {
          _input_state_fudging_helper[joy_num].stick[2].axis[0] = 0.0;
        }
      }
      else if(event->joystick.stick == 3)
      {
        joy_num = t3f_get_joystick_number(event->joystick.id);
        if(joy_num >= 0)
        {
          _input_state_fudging_helper[joy_num].stick[3].axis[0] = 0.0;
        }
      }
    }
    return;
  #endif

  #ifdef ALLEGRO_UNIX
    if(event->type == ALLEGRO_EVENT_JOYSTICK_AXIS)
    {
      if(event->joystick.stick == 1 && event->joystick.axis == 0)
      {
        joy_num = t3f_get_joystick_number(event->joystick.id);
        if(joy_num >= 0)
        {
          _input_state_fudging_helper[joy_num].stick[1].axis[0] = 0.0;
        }
      }
      else if(event->joystick.stick == 2 && event->joystick.axis == 1)
      {
        joy_num = t3f_get_joystick_number(event->joystick.id);
        if(joy_num >= 0)
        {
          _input_state_fudging_helper[joy_num].stick[2].axis[1] = 0.0;
        }
      }
    }
    return;
  #endif
}
